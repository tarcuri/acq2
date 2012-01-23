Changes:
	- Fix HTTP downloading with fs_usehomedir 1 on linux.
	- libjpeg8 support (separate patch, not in main source)
	- libpng14 support (separate patch, not in main source)

Mini FAQ:

 Sound on linux sucks:
 	- Probably SDL's fault. Make sure you compile with
	openAL support (default), make sure you have openAL
	installed, and then place these two entries in your
	config:
		- seta s_initsound 2
		- seta s_openal_driver "libopenal.so"

 The build failed on linux:
 	- Check your libjpeg and libpng versions. If you
	have libjpeg8 you will need to apply the libjpeg
	patch. If you have libpng14 you will need to apply
	the libpng patch. Both of these are in the same
	directory as the makefile. For quick reference, you
	would need to issue one or both of these before
	running make:
		- patch -p0 pngpatch.patch
		- patch -p0 jpegpatch.patch

	If this doesn't fix your issue, file a bug report in
	the issues on the cleanq2 page pretty please :)
	It will get fixed ASAP.
