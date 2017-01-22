/* NetHack 3.6	uwpfilehandler.cpp	$NHDT-Date:  $  $NHDT-Branch:  $:$NHDT-Revision:  $ */
/* Copyright (c) Bart House, 2016-2017. */
/* Nethack for the Universal Windows Platform (UWP) */
/* NetHack may be freely redistributed.  See license for details. */
#include "uwp.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Storage::Provider;
using namespace Windows::Storage::Streams;
using namespace Concurrency;

namespace Nethack
{

    FileHandler::FileHandler(void)
    {
        s_instance = this;
    }

    void FileHandler::SaveFilePicker(String ^ fileText, String ^ fileName, String ^ fileExtension)
    {
        auto delegate = [this,fileName,fileExtension]()
        {
            FileSavePicker^ picker = ref new FileSavePicker();
            picker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;

            auto extensions = ref new Platform::Collections::Vector<String ^>();
            extensions->Append(fileExtension);
            picker->FileTypeChoices->Insert("", extensions);
            picker->SuggestedFileName = fileName;

            auto pickFileTask = create_task(picker->PickSaveFileAsync());

            pickFileTask.then([this](StorageFile^ file)
            {
                m_lock.AcquireExclusive();
                m_file = file;
                m_lock.ReleaseExclusive();
                m_conditionVariable.Wake();
            });
        };

        if (m_coreDispatcher != nullptr)
        {
            m_lock.AcquireExclusive();

            auto handler = ref new Windows::UI::Core::DispatchedHandler(delegate, Platform::CallbackContext::Any);
            auto task = create_task(m_coreDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, handler));
            task.wait();

            m_conditionVariable.Sleep(m_lock);
            m_lock.ReleaseExclusive();

            if (m_file != nullptr)
            {
                auto writeTask = create_task(FileIO::WriteTextAsync(m_file, fileText)).then([]()
                {
                    // do nothing
                });
                writeTask.wait();
                m_file = nullptr;
            }
        }
    }

    String ^ FileHandler::LoadFilePicker(String ^ extension)
    {
        auto delegate = [this, extension]()
        {
            FileOpenPicker^ openPicker = ref new FileOpenPicker();
            openPicker->ViewMode = PickerViewMode::List;
            openPicker->SuggestedStartLocation = PickerLocationId::DocumentsLibrary;
            openPicker->FileTypeFilter->Append(extension);

            auto pickFileTask = create_task(openPicker->PickSingleFileAsync());
            
            pickFileTask.then([this](StorageFile^ file)
            {
                m_lock.AcquireExclusive();
                m_file = file;
                m_lock.ReleaseExclusive();
                m_conditionVariable.Wake();
            });
        };
        
        String ^ text = nullptr;

        if (m_coreDispatcher != nullptr)
        {
            m_lock.AcquireExclusive();

            auto handler = ref new Windows::UI::Core::DispatchedHandler(delegate, Platform::CallbackContext::Any);
            auto asyncAction = m_coreDispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, handler);
            auto task = concurrency::create_task(asyncAction);
            task.wait();

            m_conditionVariable.Sleep(m_lock);
            m_lock.ReleaseExclusive();

            if (m_file != nullptr)
            {
                auto readTask = create_task(FileIO::ReadTextAsync(m_file));

                readTask.wait();
                m_file = nullptr;

                text = readTask.get();
            }
        }

        return text;
    }
}