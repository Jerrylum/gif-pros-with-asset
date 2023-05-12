#include "gifclass.hpp"

/**
 * MIT License
 * Copyright (c) 2019 Theo Lemay
 * https://github.com/theol0403/gif-pros
 */

/**
 * Construct the Gif class
 * @param fname  the gif filename on the SD card (prefixed with /usd/)
 * @param parent the LVGL parent object
 */
Gif::Gif(const asset file, lv_obj_t* parent) {
	size_t len = file.size;

	_gifmem = file.buf;

	if (_gifmem != NULL) {
		// create a FILE from memory buffer
		FILE* memfp = fmemopen(_gifmem, len, "rb");

		// open gof file
		// will allocate memory for background and one animation frame.
		_gif = gd_open_gif(memfp);
		if(_gif == NULL) {
			std::cerr << "Gif::Gif - error opening gif" << std::endl;
			_cleanup();
			return;
		}

		// memory for rendering frame
		_buffer = (uint8_t*)malloc(_gif->width * _gif->height * BYTES_PER_PIXEL);
		if(_buffer == NULL) {
			// out of memory
			_cleanup();
			std::cerr << "Gif::Gif - not enough memory for frame buffer" << std::endl;
		} else {
			_cbuf = new lv_color_t[_gif->width * _gif->height];
			_canvas = lv_canvas_create(parent, NULL);
			lv_canvas_set_buffer(_canvas, _cbuf, _gif->width, _gif->height, LV_IMG_CF_TRUE_COLOR_ALPHA);
			_task = pros::c::task_create(_render_task, static_cast<void*>(this), TASK_PRIORITY_DEFAULT-2, TASK_STACK_DEPTH_DEFAULT, "GIF");
		}
	}
};


/**
 * Destructs and cleans the Gif class
 */
Gif::~Gif() {
	_cleanup();
}

/**
 * Pauses the GIF task
 */
void Gif::pause(){
	pros::c::task_suspend(_task);
}

/**
 * Resumes the GIF task
 */
void Gif::resume(){
	pros::c::task_resume(_task);
}

/**
* Deletes GIF and frees all allocated memory
*/
void Gif::clean() {
	_cleanup();
}

/**
 * Cleans and frees all allocated memory
 */
void Gif::_cleanup() {
	if(_canvas) { lv_obj_del(_canvas); _canvas = nullptr; }
	if(_cbuf) { delete[] _cbuf; _cbuf = nullptr; }
	if(_buffer) { free(_buffer); _buffer = nullptr; }
	if(_gif) { gd_close_gif(_gif); _gif = nullptr; }
	if(_gifmem) { free(_gifmem); _gifmem = nullptr; }
	// deleting task kills this thread
	if(_task) { pros::c::task_delete(_task); _task = nullptr; }
}


/**
 * Render cycle, blocks until loop count exceeds gif loop count flag (if any)
 */
void Gif::_render() {

	for (size_t looped = 1;; looped++) {
		while (gd_get_frame(_gif)) {
			int32_t now = pros::millis();

			gd_render_frame(_gif, _buffer);

			for (size_t i = 0; i < _gif->height * _gif->width; i++) {
				_cbuf[i].red = _buffer[(i * BYTES_PER_PIXEL)];
				_cbuf[i].green = _buffer[(i * BYTES_PER_PIXEL) + 1];
				_cbuf[i].blue = _buffer[(i * BYTES_PER_PIXEL) + 2];
				_cbuf[i].alpha = _buffer[(i * BYTES_PER_PIXEL) + 3];
			};

			lv_obj_invalidate(_canvas); // force canvas redraw

			int32_t delay = _gif->gce.delay * 10;
			int32_t delta = pros::millis() - now;
			delay -= delta;

			if(delay > 0) pros::delay(delay);
		}

		if(looped == _gif->loop_count) break;
		gd_rewind(_gif);
	}

	_cleanup();
}


/**
 * Calls _render()
 * @param arg Gif*
 */
void Gif::_render_task(void* arg) {
	Gif* instance = static_cast<Gif*>(arg);
	instance->_render();
}