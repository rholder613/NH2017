/* NetHack 3.6	uwpnethackmain.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace Nethack;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
NethackMain::NethackMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
    m_deviceResources(deviceResources)
{
    // Register to be notified if the Device is lost or recreated
    m_deviceResources->RegisterDeviceNotify(this);

    // Initialize text grid state
    const Nethack::Int2D & glyphPixelDimensions = DX::DeviceResources::s_deviceResources->m_asciiTexture.m_glyphPixels;
    Windows::Foundation::Size & outputSize = DX::DeviceResources::s_deviceResources->GetOutputSize();

    // TODO: Can we enforce some minimum screen size?
    assert(outputSize.Width > k_gridBorder);
    assert(outputSize.Height > k_gridBorder);

    m_gridLayoutRect = Nethack::IntRect(Nethack::Int2D(k_gridBorder, k_gridBorder), Nethack::Int2D((int)outputSize.Width - k_gridBorder, (int)outputSize.Height - k_gridBorder));

    g_textGrid.SetDeviceResources();
    g_textGrid.SetLayoutRect(m_gridLayoutRect);
    g_textGrid.ScaleAndCenter();
}

NethackMain::~NethackMain()
{
    // Deregister device notification
    m_deviceResources->RegisterDeviceNotify(nullptr);
}

void NethackMain::Suspend()
{
    // Send EOF causing a hangup to occur and exiting out of mainloop
    g_eventQueue.PushBack(Nethack::Event(EOF));
}

// Updates application state when the window size changes (e.g. device orientation change)
void NethackMain::CreateWindowSizeDependentResources() 
{
    const Nethack::Int2D & glyphPixelDimensions = DX::DeviceResources::s_deviceResources->m_asciiTexture.m_glyphPixels;
    Windows::Foundation::Size & outputSize = DX::DeviceResources::s_deviceResources->GetOutputSize();
    
    // TODO: Can we enforce some minimum screen size?
    assert(outputSize.Width > k_gridBorder);
    assert(outputSize.Height > k_gridBorder);

    Nethack::IntRect outputRect(Nethack::Int2D(k_gridBorder, k_gridBorder), Nethack::Int2D((int)outputSize.Width - k_gridBorder, (int)outputSize.Height - k_gridBorder));

    g_textGrid.SetDeviceResources();
    g_textGrid.SetLayoutRect(outputRect);
    g_textGrid.ScaleAndCenter();
    g_textGrid.CreateWindowSizeDependentResources();

}

// Updates the application state once per frame.
void NethackMain::Update() 
{
    // Update scene objects.
    m_timer.Tick([&]()
    {

#ifdef NEWCODE
        Window::s_windowListLock.AcquireShared();

        for (auto window : Window::s_windowList)
            window->Update(m_timer);

        Window::s_windowListLock.ReleaseShared();
#endif        

    });
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool NethackMain::Render() 
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return false;
    }

    auto context = m_deviceResources->GetD3DDeviceContext();

    // Reset the viewport to target the whole screen.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    // Reset render targets to the screen.
    ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
    context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

    // Clear the back buffer and depth stencil view.
    context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
    context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    g_textGrid.Render();

    return true;
}

// Notifies renderers that device resources need to be released.
void NethackMain::OnDeviceLost()
{
#ifdef NEWCODE
    Window::s_windowListLock.AcquireShared();

    for (auto window : Window::s_windowList)
        window->ReleaseDeviceDependentResources();

    Window::s_windowListLock.ReleaseShared();
#endif

    g_textGrid.ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void NethackMain::OnDeviceRestored()
{
#ifdef NEWCODE
    Window::s_windowListLock.AcquireShared();

    for (auto window : Window::s_windowList)
        window->CreateDeviceDependentResources();

    Window::s_windowListLock.ReleaseShared();
#endif

    g_textGrid.CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}


//
// Keyboard Handlers
//

void NethackMain::OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
    ScanCode scanCode = (ScanCode)args->KeyStatus.ScanCode;
    Windows::System::VirtualKey virtualKey = args->VirtualKey;

#if 0
    bool isExtendedKey = args->KeyStatus.IsExtendedKey;
    int repeatCount = args->KeyStatus.RepeatCount;
    Windows::System::VirtualKey virtualKey = args->VirtualKey;
    bool isMenuDown = args->KeyStatus.IsMenuKeyDown;
#endif

    if (scanCode >= ScanCode::Home && scanCode <= ScanCode::Delete)
    {
        Windows::UI::Core::CoreVirtualKeyStates shiftKeyState = 
                sender->GetKeyState(Windows::System::VirtualKey::Shift);
        Windows::UI::Core::CoreVirtualKeyStates controlKeyState =
                sender->GetKeyState(Windows::System::VirtualKey::Control);
        Windows::UI::Core::CoreVirtualKeyStates numLockState =
            sender->GetKeyState(Windows::System::VirtualKey::NumberKeyLock);

       bool shift = ((shiftKeyState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);
       bool control = ((controlKeyState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);
       bool numLock = ((numLockState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);

       switch (virtualKey)
       {
       case Windows::System::VirtualKey::Up:
       case Windows::System::VirtualKey::Down:
       case Windows::System::VirtualKey::Left:
       case Windows::System::VirtualKey::Right:
       case Windows::System::VirtualKey::Home:
       case Windows::System::VirtualKey::PageUp:
       case Windows::System::VirtualKey::PageDown:
       case Windows::System::VirtualKey::End:
       case Windows::System::VirtualKey::Clear:
       case Windows::System::VirtualKey::Insert:
       case Windows::System::VirtualKey::Delete:
           g_eventQueue.PushBack(Event(scanCode, shift, control));
           break;
       }

    }

    
}

void NethackMain::OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args)
{
}

void NethackMain::OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args)
{
    char c = args->KeyCode;
    if (c == '\r') c = '\n';
    g_eventQueue.PushBack(Event(c));
}

void NethackMain::OnAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher ^ dispatcher, Windows::UI::Core::AcceleratorKeyEventArgs ^ args)
{
    Windows::UI::Core::CoreWindow^ sender = Windows::UI::Core::CoreWindow::GetForCurrentThread();
    Windows::System::VirtualKey virtualKey = args->VirtualKey;
    bool isExtended = args->KeyStatus.IsExtendedKey;
    bool wasKeyDown = args->KeyStatus.WasKeyDown;
    Windows::UI::Core::CoreAcceleratorKeyEventType eventType = args->EventType;
    Windows::UI::Core::CoreVirtualKeyStates menuState = sender->GetKeyState(Windows::System::VirtualKey::Menu);
    ScanCode scanCode = (ScanCode)args->KeyStatus.ScanCode;
    Windows::UI::Core::CoreVirtualKeyStates shiftKeyState =
        sender->GetKeyState(Windows::System::VirtualKey::Shift);
    Windows::UI::Core::CoreVirtualKeyStates controlKeyState =
        sender->GetKeyState(Windows::System::VirtualKey::Control);
    bool shift = ((shiftKeyState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);
    bool control = ((controlKeyState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);

    bool menu = ((menuState & Windows::UI::Core::CoreVirtualKeyStates::Down) == Windows::UI::Core::CoreVirtualKeyStates::Down);

    if (menu && virtualKey != Windows::System::VirtualKey::Menu && eventType == Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown)
    {
        g_eventQueue.PushBack(Event(scanCode, shift, control, menu));
        args->Handled = true;
        return;
    }


#if 0
    if (virtualKey == Windows::System::VirtualKey::Menu && eventType == Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyDown)
    {
        args->Handled = true;
        return;
    }

    if (virtualKey == Windows::System::VirtualKey::Menu && eventType == Windows::UI::Core::CoreAcceleratorKeyEventType::SystemKeyUp)
    {
        args->Handled = true;
        return;
    }
#endif

}

//
// Gesture Event Handlers
//

void NethackMain::OnPointerPressed(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args)
{
#if 0
    unsigned int pointerId = args->CurrentPoint->PointerId;

    Windows::UI::Input::PointerPoint^ pointerPoint = Windows::UI::Input::PointerPoint::GetCurrentPoint(pointerId);

    Float2D screenPosition(pointerPoint->RawPosition.X, pointerPoint->RawPosition.Y);

    for (auto textGrid : m_textGrids)
    {
        if (textGrid->HitTest(screenPosition))
        {
            m_hitTextGrid = textGrid;
            gestureRecognizer->ProcessDownEvent(pointerPoint);
            return;
        }
    }
#else
    unsigned int pointerId = args->CurrentPoint->PointerId;

    Windows::UI::Input::PointerPoint^ pointerPoint = Windows::UI::Input::PointerPoint::GetCurrentPoint(pointerId);

    Float2D screenPosition(pointerPoint->RawPosition.X, pointerPoint->RawPosition.Y);

    if (Nethack::g_textGrid.HitTest(screenPosition))
    {
        gestureRecognizer->ProcessDownEvent(pointerPoint);
    }
#endif

}

void NethackMain::OnPointerMoved(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args)
{
    unsigned int pointerId = args->CurrentPoint->PointerId;

    Windows::Foundation::Collections::IVector<Windows::UI::Input::PointerPoint^>^ pointerPoints =
        Windows::UI::Input::PointerPoint::GetIntermediatePoints(pointerId);

    gestureRecognizer->ProcessMoveEvents(pointerPoints);

}

void NethackMain::OnPointerReleased(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args)
{
    unsigned int pointerId = args->CurrentPoint->PointerId;

    Windows::UI::Input::PointerPoint^ pointerPoint = Windows::UI::Input::PointerPoint::GetCurrentPoint(
        pointerId);

    gestureRecognizer->ProcessUpEvent(pointerPoint);

    //    m_hitTextGrid = nullptr;
}

void NethackMain::OnPointerWheelChanged(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args)
{
    unsigned int pointerId = args->CurrentPoint->PointerId;

    // Create transformed PointerPoint relative to parent object
    Windows::UI::Input::PointerPoint^ pointerPoint = Windows::UI::Input::PointerPoint::GetCurrentPoint(pointerId);

    // Set mouse wheel parameters
    //    Windows::UI::Input::MouseWheelParameters^ params = m_gestureRecognizer->MouseWheelParameters;
    //    Windows::Foundation::Size viewSize = ViewSize();
    //    params->CharTranslation = Windows::Foundation::Point(0.05f * viewSize.Width, 0.05f * viewSize.Height);
    //    params->PageTranslation = Windows::Foundation::Point(viewSize.Width, viewSize.Height);
    //    params->DeltaScale = 1.5f;
    //    params->DeltaRotationAngle = 22.5f;

    // Assign wheel pointer to background.
    Windows::System::VirtualKeyModifiers vkmod = args->KeyModifiers;
    gestureRecognizer->ProcessMouseWheelEvent(
        pointerPoint,
        (int)(vkmod & Windows::System::VirtualKeyModifiers::Shift) != 0,
        (int)(vkmod & Windows::System::VirtualKeyModifiers::Control) != 0);
}

void NethackMain::OnHolding(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::HoldingEventArgs^ args)
{

}

void NethackMain::ProcessTap(Float2D &screenPosition, Event::Tap tap)
{
    Int2D gridPosition;

    if (Nethack::g_textGrid.HitTest(screenPosition, gridPosition))
    {
        gridPosition.m_x++;
        gridPosition.m_y--;
        Nethack::g_eventQueue.PushBack(Nethack::Event(gridPosition, tap));
        // TODO: we should get passed down whether it it a left, right or middle click
        // NHEVENT_MS(CLICK_1, gridPosition.m_x + 1, gridPosition.m_y - 4);
    }
}

void NethackMain::OnRightTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::RightTappedEventArgs^ args)
{
    Float2D screenPosition(DX::ConvertDipsToPixels(args->Position.X, m_deviceResources->GetDpi()),
        DX::ConvertDipsToPixels(args->Position.Y, m_deviceResources->GetDpi()));

    ProcessTap(screenPosition, Event::Tap::Right);
}

void NethackMain::OnTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::TappedEventArgs^ args)
{
    Float2D screenPosition(DX::ConvertDipsToPixels(args->Position.X, m_deviceResources->GetDpi()),
        DX::ConvertDipsToPixels(args->Position.Y, m_deviceResources->GetDpi()));

    ProcessTap(screenPosition, Event::Tap::Left);
}

void NethackMain::OnManipulationStarted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationStartedEventArgs^ args)
{
    m_flickDirection = Direction::None;
}


NethackMain::Direction NethackMain::AngleToDirection(float inAngle)
{
    assert(inAngle <= 180.0 && inAngle >= -180.0);

    inAngle += 180.0;   // 0 - 360 -> 0:left, 90:down, 180:right, 270: up, 360 left
    inAngle /= 45.0;    // 0 - 8 -> 0:left, 2:down, 4:right, 6:up, 8: left 

    int direction = (int)round(inAngle);

    if (direction == 8) direction = 0;

    return (Direction)direction;

}


void NethackMain::OnManipulationUpdated(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs^ args)
{
    float dx = args->Cumulative.Translation.X;
    float dy = args->Cumulative.Translation.Y;
    float angle = (360.0f * atan2f(dy, dx)) / (2.0f * (float)M_PI);

    m_flickDirection = AngleToDirection(angle);
}

void NethackMain::OnManipulationInertiaStarting(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationInertiaStartingEventArgs^ args)
{

}

void NethackMain::OnManipulationCompleted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationCompletedEventArgs^ args)
{
    if (m_flickDirection != Direction::None)
    {
        static char directionToKey[] = { '4', '7', '8', '9', '6', '3', '2', '1' };
        char key = directionToKey[(int)m_flickDirection];

        Nethack::g_eventQueue.PushBack(Nethack::Event(key));
    }
}

extern "C" { void mainloop(const char * localDir, const char * installDir); }

void NethackMain::MainLoop(void)
{
    std::wstring localDirW = Windows::Storage::ApplicationData::Current->LocalFolder->Path->Data();
    std::string localDir(localDirW.begin(), localDirW.end());

    std::wstring installDirW = Windows::ApplicationModel::Package::Current->InstalledLocation->Path->Data();
    std::string installDir(installDirW.begin(), installDirW.end());

    mainloop(localDir.c_str(), installDir.c_str());
}

