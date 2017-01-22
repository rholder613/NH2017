/* NetHack 3.6	uwpnethackmain.h	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#pragma once

// Renders Direct2D and 3D content on the screen.
namespace Nethack
{
    class NethackMain : public DX::IDeviceNotify
    {
    public:
        NethackMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
        ~NethackMain();
        void CreateWindowSizeDependentResources();
        void Update();
        bool Render();
        void Suspend();
        void MainLoop();

        // IDeviceNotify
        virtual void OnDeviceLost();
        virtual void OnDeviceRestored();

        void Attach(_In_ Windows::UI::Input::GestureRecognizer^ gestureRecognizer);

        // Gesture event handlers
        void OnPointerPressed(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerMoved(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerReleased(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args);
        void OnPointerWheelChanged(Windows::UI::Input::GestureRecognizer^ gestureRecognizer, Windows::UI::Core::PointerEventArgs^ args);

        void OnHolding(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::HoldingEventArgs^ args);
        void OnTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::TappedEventArgs^ args);
        void OnRightTapped(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::RightTappedEventArgs^ args);
        void OnManipulationStarted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationStartedEventArgs^ args);
        void OnManipulationUpdated(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationUpdatedEventArgs^ args);
        void OnManipulationInertiaStarting(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationInertiaStartingEventArgs^ args);
        void OnManipulationCompleted(Windows::UI::Input::GestureRecognizer^ sender, Windows::UI::Input::ManipulationCompletedEventArgs^ args);

        // Keyboard
        void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
        void OnCharacterReceived(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CharacterReceivedEventArgs^ args);
        void OnAcceleratorKeyActivated(Windows::UI::Core::CoreDispatcher ^ dispatcher, Windows::UI::Core::AcceleratorKeyEventArgs ^ args);

    private:

        const int k_gridBorder = 16;

        IntRect m_gridLayoutRect;

        // Cached pointer to device resources.
        std::shared_ptr<DX::DeviceResources> m_deviceResources;

        // Rendering loop timer.
        DX::StepTimer m_timer;

        enum class Direction { Left, LeftUp, Up, RightUp, Right, RightDown, Down, LeftDown, None };

        Direction m_flickDirection;

        Direction AngleToDirection(float inAngle);

        void ProcessTap(Float2D &screenPosition, Nethack::Event::Tap tap);

    };
}
