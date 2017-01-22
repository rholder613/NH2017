/* NetHack 3.6	uwpapp.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace Nethack;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace Windows::System::Threading;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
#ifdef _DEBUG
    // We don't want to taint the application usage information with development builds
    // development builds should have build version 0
    auto package = Windows::ApplicationModel::Package::Current;
    auto packageId = package->Id;
    auto version = packageId->Version;
    auto build = version.Build;
    assert(build == 0);
#endif

    auto direct3DApplicationSource = ref new Direct3DApplicationSource();
    CoreApplication::Run(direct3DApplicationSource);
    return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new App();
}

App::App() :
    m_windowClosed(false),
    m_windowVisible(true),
    m_exit(false),
    m_loopCount(0)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

    CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

    // At this point we have access to the device. 
    // We can create the device-dependent resources.
    m_deviceResources = std::make_shared<DX::DeviceResources>();
    DX::DeviceResources::s_deviceResources = m_deviceResources;

    m_gestureRecognizer = ref new Windows::UI::Input::GestureRecognizer();

    Attach(m_gestureRecognizer);

    m_fileHandler = ref new Nethack::FileHandler();

    // Start worker thread which will run the nethack main loop
    auto workItemHandler = ref new WorkItemHandler([this](IAsyncAction^)
    {
        RunNethackMainLoop();
    });

    m_nethackWorker = ThreadPool::RunAsync(workItemHandler, WorkItemPriority::High, WorkItemOptions::TimeSliced);
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
    window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

    DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

    currentDisplayInformation->DpiChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

    currentDisplayInformation->OrientationChanged +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

    DisplayInformation::DisplayContentsInvalidated +=
        ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

    window->PointerPressed +=
        ref new TypedEventHandler<CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerPressed);

    window->PointerMoved +=
        ref new TypedEventHandler<CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerMoved);

    window->PointerReleased +=
        ref new TypedEventHandler<CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerReleased);

    window->PointerWheelChanged +=
        ref new TypedEventHandler<CoreWindow^, Windows::UI::Core::PointerEventArgs^>(this, &App::OnPointerWheelChanged);

    // keyboard event handlers
    window->KeyDown +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyDown);
    window->KeyUp +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &App::OnKeyUp);
    window->CharacterReceived +=
        ref new TypedEventHandler<CoreWindow^, CharacterReceivedEventArgs^>(this, &App::OnCharacterReceived);

    // Disable all pointer visual feedback for better performance when touching.
    auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
    pointerVisualizationSettings->IsContactFeedbackEnabled = false;
    pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;

    m_deviceResources->SetWindow(window);
    m_fileHandler->SetCoreDispatcher(window->Dispatcher);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
    if (m_main == nullptr)
    {
        m_main = std::unique_ptr<NethackMain>(new NethackMain(m_deviceResources));
    }
}

// This method is called after the window becomes active.
void App::Run()
{
    CoreWindow::GetForCurrentThread()->Dispatcher->AcceleratorKeyActivated +=
        ref new TypedEventHandler<Windows::UI::Core::CoreDispatcher ^, Windows::UI::Core::AcceleratorKeyEventArgs ^>(this, &App::OnAcceleratorKeyActivated);

    while (!m_windowClosed)
    {
        if (m_windowVisible)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            m_main->Update();

            if (m_main->Render())
            {
                m_deviceResources->Present();
            }
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }

        // TODO: Find out the graceful way to exit
        if (m_exit) exit(0);
    }
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
    int loopCount = m_loopCount;

    m_main->Suspend();

    // Save app state asynchronously after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely. After about five seconds,
    // the app will be forced to exit.
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

    create_task([this, deferral, loopCount]()
    {
        m_deviceResources->Trim();

        while (m_loopCount == loopCount) Sleep(100);

        deferral->Complete();
    });
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
    // Restore any data or state that was unloaded on suspend. By default, data
    // and state are persisted when resuming from suspend. Note that this event
    // does not occur if the app was previously terminated.

    // Insert your code here.
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
    m_deviceResources->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
    m_main->CreateWindowSizeDependentResources();
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    m_windowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
    // Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
    // if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
    // you should always retrieve it using the GetDpi method.
    // See DeviceResources.cpp for more details.
    m_deviceResources->SetDpi(sender->LogicalDpi);
    m_main->CreateWindowSizeDependentResources();
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
    m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
    m_main->CreateWindowSizeDependentResources();
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
    m_deviceResources->ValidateDevice();
}

//
// Pointer event handlers
//

void App::OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnPointerPressed(m_gestureRecognizer, args);
}

void App::OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnPointerMoved(m_gestureRecognizer, args);
}

void App::OnPointerReleased(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnPointerReleased(m_gestureRecognizer, args);
}

void App::OnPointerWheelChanged(Windows::UI::Core::CoreWindow^, Windows::UI::Core::PointerEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnPointerWheelChanged(m_gestureRecognizer, args);
}

//
// Keyboard support
//

void App::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnKeyDown(sender, args);
}

void App::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnKeyUp(sender, args);
}

void App::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnCharacterReceived(sender, args);
}

void App::OnAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher ^ dispatcher, Windows::UI::Core::AcceleratorKeyEventArgs ^ args)
{
    if (m_main != nullptr)
        m_main->OnAcceleratorKeyActivated(dispatcher, args);
}



//
// Gesture support
//

void App::Attach(_In_ Windows::UI::Input::GestureRecognizer^ gestureRecognizer)
{
    gestureRecognizer->GestureSettings =
        Windows::UI::Input::GestureSettings::Hold |
        Windows::UI::Input::GestureSettings::HoldWithMouse |
        Windows::UI::Input::GestureSettings::Tap |
        Windows::UI::Input::GestureSettings::RightTap |
        Windows::UI::Input::GestureSettings::ManipulationTranslateX |
        Windows::UI::Input::GestureSettings::ManipulationTranslateY |
        Windows::UI::Input::GestureSettings::ManipulationScale;

    // Register all the delegates
    m_tokenTapped = gestureRecognizer->Tapped::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::TappedEventArgs^ >(this, &App::OnTapped));

    m_tokenRightTapped = gestureRecognizer->RightTapped::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::RightTappedEventArgs^ >(this, &App::OnRightTapped));

    m_tokenHolding = gestureRecognizer->Holding::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::HoldingEventArgs^ >(this, &App::OnHolding));

    m_tokenManipulationStarted = gestureRecognizer->ManipulationStarted::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::ManipulationStartedEventArgs^ >(this, &App::OnManipulationStarted));

    m_tokenManipulationUpdated = gestureRecognizer->ManipulationUpdated::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::ManipulationUpdatedEventArgs^ >(this, &App::OnManipulationUpdated));

    m_tokenManipulationInertiaStarting = gestureRecognizer->ManipulationInertiaStarting::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::ManipulationInertiaStartingEventArgs^ >(this, &App::OnManipulationInertiaStarting));

    m_tokenManipulationCompleted = gestureRecognizer->ManipulationCompleted::add(
        ref new Windows::Foundation::TypedEventHandler <
        Windows::UI::Input::GestureRecognizer^, Windows::UI::Input::ManipulationCompletedEventArgs^ >(this, &App::OnManipulationCompleted));
}

void App::OnHolding(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::HoldingEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnHolding(sender, args);
}

void App::OnTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::TappedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnTapped(sender, args);
}

void App::OnRightTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::RightTappedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnRightTapped(sender, args);
}

void App::OnManipulationStarted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationStartedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnManipulationStarted(sender, args);
}

void App::OnManipulationUpdated(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnManipulationUpdated(sender, args);
}

void App::OnManipulationInertiaStarting(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationInertiaStartingEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnManipulationInertiaStarting(sender, args);
}

void App::OnManipulationCompleted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationCompletedEventArgs^ args)
{
    if (m_main != nullptr)
        m_main->OnManipulationCompleted(sender, args);
}

//
//
//

void App::RunNethackMainLoop(void)
{
    while (1)
    {
        m_main->MainLoop();
        m_loopCount++;
    }

#if 0
    // TODO: We should just call back into mainloop again but Nethack can not handle getting started again ... need to force exit
    m_exit = true;

    while (1) Sleep(1000);
#endif

}
