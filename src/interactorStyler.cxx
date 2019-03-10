/****************************************************************************
*   interactorStyler.cxx
*
*   Created by:     Michael Kuczynski
*   Created on:     19/01/2019
*   Description:    Custom interactor styling implementation.
****************************************************************************/

#include "interactorStyler.hxx"

void myInteractorStyler::setImageViewer( vtkImageMapper* imageMapper, vtkRenderWindow* renderWindow )
{
    _RenderWindow = renderWindow;
    _ImageMapper = imageMapper;
    minSlice = imageMapper->GetWholeZMin();
    maxSlice = imageMapper->GetWholeZMax();
    windowLevel = imageMapper->GetColorLevel();
    window = imageMapper->GetColorWindow();
    
    // Start current slice at 0
    slice = minSlice;
}

void myInteractorStyler::setSliceStatusMapper( vtkTextMapper* statusMapper ) 
{
    _SliceStatusMapper = statusMapper;
}

void myInteractorStyler::setWindowLevelStatusMapper( vtkTextMapper* statusMapper ) 
{
    _WindowLevelStatusMapper = statusMapper;
}

void myInteractorStyler::setWindowStatusMapper( vtkTextMapper* statusMapper ) 
{
    _WindowStatusMapper = statusMapper;
}

void myInteractorStyler::moveSliceForward() 
{
    if ( slice < maxSlice ) 
    {
        slice += 1;

        _ImageMapper->SetZSlice( slice );

        // Create the message to be displayed.
        std::string msg = ImageMessage::sliceNumberFormat( slice, maxSlice );

        // Update the mapper and render.
        _SliceStatusMapper->SetInput( msg.c_str() );
        _RenderWindow->Render();
    }
}

void myInteractorStyler::moveSliceBackward() 
{
    if ( slice > minSlice ) 
    {
        slice -= 1;

        _ImageMapper->SetZSlice( slice );

        // Create the message to be displayed.
        std::string msg = ImageMessage::sliceNumberFormat( slice, maxSlice );

        // Update the mapper and render.
        _SliceStatusMapper->SetInput( msg.c_str() );
        _RenderWindow->Render();
    }
}

void myInteractorStyler::moveWindowLevelForward() 
{
    windowLevel += 10;

    _ImageMapper->SetColorLevel( windowLevel );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowLevelFormat( int( windowLevel ) );

    // Update the mapper and render.
    _WindowLevelStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowLevelBackward() 
{
    windowLevel -= 10;

    _ImageMapper->SetColorLevel( windowLevel );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowLevelFormat( int( windowLevel ) );

    // Update the mapper and render.
    _WindowLevelStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowForward()
{
    window += 10;

    _ImageMapper->SetColorWindow( window );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowFormat( int( window ) );

    // Update the mapper and render.
    _WindowStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();
}

void myInteractorStyler::moveWindowBackward()
{
    window -= 10;

    _ImageMapper->SetColorWindow( window );

    // Create the message to be displayed.
    std::string msg = ImageMessage::windowFormat( int( window ) );

    // Update the mapper and render.
    _WindowStatusMapper->SetInput( msg.c_str() );
    _RenderWindow->Render();  
}