#pragma once

namespace AV{
	class Window{
	protected:
		int _width;
		int _height;

		bool _open;

	public:
		Window();
        virtual ~Window() = 0;
		virtual void update() = 0;
        virtual bool open() = 0;

		virtual bool close() = 0;
        
        virtual bool isOpen() = 0;
        
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
