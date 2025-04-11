package lime.media.openal.ext;

class EXT_int32 {
	@:isVar public static var FORMAT_MONO_INT32(get, null):Int = -1;
	@:isVar public static var FORMAT_STEREO_INT32(get, null):Int = -1;

	static function get_FORMAT_MONO_INT32() {
		if (FORMAT_MONO_INT32 == -1)
			FORMAT_MONO_INT32 = AL.getEnumValue("AL_FORMAT_MONO_I32");
		return FORMAT_MONO_INT32;
	}

	static function get_FORMAT_STEREO_INT32() {
		if (FORMAT_STEREO_INT32 == -1)
			FORMAT_STEREO_INT32 = AL.getEnumValue("AL_FORMAT_STEREO_I32");
		return FORMAT_STEREO_INT32;
	}
}