#include "MacOSUtils.h"

#import <AppKit/NSWindow.h>
#include <SDL.h>

namespace AV
{
    
    unsigned long WindowContentViewHandle(const SDL_SysWMinfo &info)
    {
        NSWindow *window = info.info.cocoa.window;
        NSView *view = [window contentView];
        return (unsigned long)view;
    }
    
}

