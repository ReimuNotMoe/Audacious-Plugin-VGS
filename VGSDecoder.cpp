/*
    This file is part of Audacious-Plugin-VGS.
    Copyright (C) 2016, 2017, 2020 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <vector>

#include "VGSDecoder.hpp"

EXPORT VGSMMLDec aud_plugin_instance;

const char *const VGSMMLDec::exts[] = { "bgm", "mml", "vgs", nullptr };

inline static int DetectFileType(const char *filename){
	if (!filename)
		return InputFileType_Bad;

	size_t fn_len = strlen(filename);

	if ( fn_len < 4 )
		return InputFileType_Bad;

	uint8_t buf_ext[4];

	/* aa.bgm
	 *       ^
	 *   ^   filename+fn_len-3
	 * ^ filename+fn_len-3
	 * filename
	 */

	memcpy(buf_ext,filename+fn_len-3,4);

	int i;

	for(i=0;i<3;i++) buf_ext[i] = tolower(buf_ext[i]);

	if (memcmp(buf_ext,"bgm",3) == 0) return InputFileType_BGM;
	if (memcmp(buf_ext,"mml",3) == 0) return InputFileType_MML;
	if (memcmp(buf_ext,"vgs",3) == 0) return InputFileType_VGS;

	return InputFileType_Bad;
}

inline static void *StripFileURIPrefix(const char *filename){
	size_t fnlen = strlen(filename);

	if ( fnlen < 8 ) return nullptr;

	void *ret = malloc(fnlen);

	/*
	 * file://a
	 *       ^
	 *       7
	 */

	memcpy(ret,filename+7,fnlen-7);

	return ret;
}

static struct VgsMetaData *MetaData2MetaData(size_t InputLen, uint8_t *InputData){
	struct VgsMetaData *ret = (struct VgsMetaData *)calloc(1,sizeof(struct VgsMetaData));

	uint8_t *curpos = InputData;
	uint8_t *tmppos = NULL;
	uint8_t intbuf[16] = {0};

	// Year

	tmppos =(uint8_t *)memmem(InputData,InputLen,"year",4);

	if ( tmppos ) curpos = (uint8_t *)memmem(tmppos,(InputData+InputLen-tmppos),"=",1);

	if ( curpos ) tmppos = (uint8_t *)memmem(curpos,(InputData+InputLen-curpos),"\n",1);

	if ( !tmppos ) tmppos = (uint8_t *)memmem(curpos,(InputData+InputLen-curpos),"\r\n",2);

	if ( tmppos ) (uint8_t *)memcpy(intbuf,curpos+1,tmppos-curpos+1);

	ret->year = atoi((const char *)intbuf);

	std::cout << "meta - year: " << ret->year << "\n";

	// TODO

	// Too late, sleep first.(2016-02-11 02:17:53 +0800)


	return ret;

}

static struct VgsMetaHeader MetaData2MetaHeader(size_t InputLen, uint8_t *InputData){




}

void *CreateCTXFromFileData(int InputType, Index<char>& __file_data){
	void *vgs_ctx = vgsdec_create_context();

	if (!vgs_ctx) {
		AUDERR("vgsdec: vgsdec_create_context error");
		return nullptr;
	}

	struct VgsBgmData* compiled_bgm = nullptr;
	uint8_t *bgm_data;
	size_t bgm_len;

	if ( InputType == InputFileType_MML ) {
		struct VgsMmlErrorInfo vmc_err;

		__file_data.append(0);

		compiled_bgm = vgsmml_compile_from_memory(&__file_data[0], __file_data.len(), &vmc_err);

		if (compiled_bgm) {
			bgm_data = (uint8_t *)compiled_bgm->data;
			bgm_len = compiled_bgm->size;
		} else {
			if (vmc_err.line) {
				AUDERR("vgsdec: error(%d) line=%d: %s\n", vmc_err.code, vmc_err.line, vmc_err.message);
			} else {
				AUDERR("vgsdec: error(%d) %s\n", vmc_err.code, vmc_err.message);
			}

			return nullptr;
		}
	} else {
		bgm_data = (uint8_t *)&__file_data[0];
		bgm_len = __file_data.len();
	}

	if (vgsdec_load_bgm_from_memory(vgs_ctx, bgm_data, bgm_len)) {
		AUDERR("vgsdec: vgs bgm load error");
		vgsdec_release_context(vgs_ctx);
		return nullptr;
	}

	if (compiled_bgm)
		vgsmml_free_bgm_data(compiled_bgm);

	return vgs_ctx;

}

bool VGSMMLDec::init() {
	return true;
}

void VGSMMLDec::cleanup() {

}

bool VGSMMLDec::is_our_file(const char *filename, VFSFile &file) {
	return DetectFileType(filename) != 0;
}


bool VGSMMLDec::play(const char *filename, VFSFile &file) {
	AUDDBG("vgsdec: play called");

	auto file_data = file.read_all();

	int filetype = DetectFileType(filename);
	AUDDBG("vgsdec: file type: %d", filetype);
	if ( !filetype ) return false;


	void *vgs_ctx = CreateCTXFromFileData(filetype, file_data);
	AUDDBG("vgsdec: file loaded");

	std::vector<uint8_t> sample_buffer(44100);

	open_audio(FMT_S16_LE, 22050, 1);

	int seek_value = 0;

	while(vgsdec_get_value(vgs_ctx, VGSDEC_REG_PLAYING) && vgsdec_get_value(vgs_ctx, VGSDEC_REG_LOOP_COUNT) == 0) {

		if (check_stop())
			break;

		seek_value = check_seek();

		if (seek_value >= 0) {
			AUDDBG("vgsdec: [normpl] seeking to %d", seek_value / 1000);
			vgsdec_set_value(vgs_ctx, VGSDEC_REG_TIME, seek_value / 1000 * 22050);
		}

		vgsdec_execute(vgs_ctx, sample_buffer.data(), 44100);
		write_audio(sample_buffer.data(), 44100);
	}

	/* wait the end of fadeout if looped */
	if (vgsdec_get_value(vgs_ctx, VGSDEC_REG_LOOP_COUNT)) {
		vgsdec_set_value(vgs_ctx, VGSDEC_REG_FADEOUT, 1);
		while(vgsdec_get_value(vgs_ctx, VGSDEC_REG_PLAYING)) {
			if (check_stop())
				break;

			seek_value = check_seek();

			if (seek_value >= 0) {
				AUDDBG("vgsdec: [fading] seeking to %d", seek_value / 1000);
				vgsdec_set_value(vgs_ctx,VGSDEC_REG_TIME,seek_value / 1000 * 22050);
			}
			vgsdec_execute(vgs_ctx, sample_buffer.data(), 44100);
			write_audio(sample_buffer.data(), 44100);
		}
	}

	vgsdec_release_context(vgs_ctx);


	AUDDBG("vgsdec: play done");

	return true;
}

const char VGSMMLDec::about[] =
	N_("VGS BGM/MML Decoder v0.3\n\n"
	   "\n"
	   "\n\n"
	   "Based on vgs-bgm-decoder by Yoji Suzuki:\n"
	   "https://github.com/suzukiplan/vgs-bgm-decoder/");

bool VGSMMLDec::read_tag(const char *filename, VFSFile &file, Tuple &tuple, Index<char> *image) {
	AUDDBG("vgsdec: reading song info for: %s", filename);

	tuple.set_filename(filename);

	auto file_data = file.read_all();

	int filetype = DetectFileType(filename);
	AUDDBG("vgsdec: file type: %d", filetype);
	if ( !filetype ) return false;

	void *vgs_ctx = CreateCTXFromFileData(filetype, file_data);
	AUDDBG("vgsdec: file loaded");

	tuple.set_str(Tuple::Quality, _("sequenced"));

	if ( vgs_ctx ) {
		// TODO: Proper playback length handling(with additional loops)

		int playlen = vgsdec_get_value(vgs_ctx, VGSDEC_REG_TIME_LENGTH) / 22050 * 1000;

		playlen += 6000;
		tuple.set_int(Tuple::Length, playlen);

		if ( filetype == InputFileType_BGM ) {
			tuple.set_str(Tuple::Codec, "Video Game System mk-II SR Compiled Music Macro Language");
		} else if ( filetype == InputFileType_VGS ) {
			tuple.set_str(Tuple::Codec, "Video Game System mk-II SR Packed Music Macro Language");

			struct VgsMetaData *thismeta;

			thismeta = vgsdec_get_meta_data(vgs_ctx,0);

			tuple.set_int(Tuple::Year,thismeta->year);
			tuple.set_int(Tuple::Track,thismeta->track);
			tuple.set_str(Tuple::Album,thismeta->album);
			tuple.set_str(Tuple::Title,thismeta->song);
			tuple.set_str(Tuple::Artist,thismeta->creator);
			tuple.set_str(Tuple::AlbumArtist,thismeta->team);

		} else if ( filetype == InputFileType_MML ) {
			tuple.set_str(Tuple::Codec, "Video Game System mk-II SR Raw Music Macro Language");
		}

		vgsdec_release_context(vgs_ctx);
	}

	tuple.set_int(Tuple::Bitrate, 352);

	return true;
}

