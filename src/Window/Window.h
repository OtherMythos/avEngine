#pragma once

namespace AV{
	class Window{
	protected:
		int _width;
		int _height;

		bool _open;

	public:
		Window();
        virtual ~Window();
		virtual void update();
        virtual bool open();

		virtual void close();
        
        /**
         @return Whether or not the window is open
         */
        bool isOpen() { return _open; };
        
        /**
         @return The width of the window.
         */
        int getWidth() { return _width; };
        /**
         @return The height of the window.
         */
        int getHeight() { return _height; };
	};
}
