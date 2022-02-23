#include "record_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

 int RecordStream::init_stream_files(void)
{
	int i, j;

	{
		stream_files.fd = -1;
		stream_files.fd_info = -1;
		stream_files.session_id = -1;
		stream_files.gop_structure = 0;
		stream_files.fast_seek_intvl = 0;
		stream_files.fd_fast_seek = -1;
		stream_files.total_bytes = 0;
		for (j = 0; j < MAX_SVCT_LAYERS; ++j)
		{
			stream_files.fd_svct[j] = -1;
		}
	}
	return 0;
}

 int RecordStream::close_stream_files()
{
	int i;

	if (stream_files.fd > 0)
	{
		close(stream_files.fd);
		stream_files.fd = -1;
	}
	if (stream_files.fd_info > 0)
	{
		close(stream_files.fd);
		stream_files.fd_info = -1;
	}
	if (split_svct_layer_flag)
	{
		for (i = 0; i < MAX_SVCT_LAYERS; ++i)
		{
			if (stream_files.fd_svct[i] > 0)
			{
				stream_files.fd_svct[i] = -1;
			}
		}
		if (stream_files.gop_structure > 0)
		{
			stream_files.gop_structure = 0;
		}
	}
	if (split_fast_seek_flag)
	{
		if (stream_files.fd_fast_seek > 0)
		{
			stream_files.fd_fast_seek = -1;
		}
		if (stream_files.fast_seek_intvl > 0)
		{
			stream_files.fast_seek_intvl = 0;
		}
	}
	return 0;
}

 int RecordStream::deinit_stream_files()
{
	int i;

	close_stream_files();
	return 0;
}

// return 0 if it's not new session,  return 1 if it is new session
 int RecordStream::is_new_session(struct iav_framedesc *framedesc)
{
	int new_session = 0;
	if (framedesc->session_id != stream_files.session_id)
	{
		// a new session
		new_session = 1;
	}
	if (file_size_flag)
	{
		if ((stream_files.total_bytes / 1024) > (file_size_mega_byte * 1024))
			new_session = 1;
	}

	return new_session;
}

 u8 RecordStream::is_new_frame(struct iav_framedesc *framedesc)
{
	u8 new_frame = 0;
	struct iav_stream_cfg streamcfg;
	int slice_per_info = IAV_ONE_BITS_INFO_PER_TILE;

	if (framedesc->stream_type == IAV_STREAM_TYPE_H265)
	{
		streamcfg.id = framedesc->id;
		streamcfg.cid = IAV_H265_CFG_SLICE;
		ioctl(fd_iav, IAV_IOC_GET_STREAM_CONFIG, &streamcfg);
		slice_per_info = streamcfg.arg.h265_slice.slices_per_info;
	}
	else
	{
		new_frame = 1;
		return new_frame;
	}

	switch (slice_per_info)
	{
	case IAV_ONE_BITS_INFO_PER_TILE:
		if ((framedesc->tile_id == 0) && (framedesc->slice_id == 0))
		{
			new_frame = 1;
		}
		break;
	case IAV_ONE_BITS_INFO_PER_FRAME:
		if ((framedesc->tile_id == framedesc->tile_num - 1) &&
			(framedesc->slice_id == framedesc->slice_num - 1))
		{
			new_frame = 1;
		}
		break;
	default:
		if ((framedesc->slice_id == slice_per_info - 1))
		{
			new_frame = 1;
		}
		break;
	}

	return new_frame;
}

u8 RecordStream::is_last_framedesc(struct iav_framedesc *framedesc)
{
	u8 is_lastdesc = 0;

	if (framedesc->stream_type == IAV_STREAM_TYPE_H265)
	{
		if ((framedesc->tile_id == framedesc->tile_num - 1) &&
			(framedesc->slice_id == framedesc->slice_num - 1))
		{
			is_lastdesc = 1;
		}
	}
	else
	{
		is_lastdesc = 1;
	}

	return is_lastdesc;
}

 int RecordStream::get_time_string(char *time_str, int len)
{
	time_t t;
	struct tm *tmp;

	t = time(NULL);
	tmp = gmtime(&t);
	if (strftime(time_str, len, "%m%d%H%M%S", tmp) == 0)
	{
		printf("date string format error \n");
		return -1;
	}

	return 0;
}


 int RecordStream::write_frame_info_header()
{
	char dummy_config[36];
	int version = VERSION;
	u32 size = sizeof(dummy_config);
	int fd_info = stream_files.fd_info;
	
	snprintf(dummy_config, sizeof(dummy_config), "Here should contain H264 config\n");

	if (write(fd_info, &version, sizeof(int)) < 0 ||
		write(fd_info, &size, sizeof(u32)) < 0 ||
		write(fd_info, &dummy_config, sizeof(dummy_config)) < 0)
	{
		perror("write_data(4)");
		return -1;
	}

	return 0;
}

 int RecordStream::write_frame_info(struct iav_framedesc *framedesc)
{
	typedef struct video_frame_s
	{
		u32 size;
		u32 pts;
		u32 pic_type;
		u32 reserved;
	} video_frame_t;
	video_frame_t frame_info;
	frame_info.pic_type = framedesc->pic_type;
	frame_info.pts = (u32)framedesc->dsp_pts;
	frame_info.size = framedesc->size;
	int stream_id = framedesc->id;
	int fd_info = stream_files.fd_info;
	
	if (write(fd_info, &frame_info, sizeof(frame_info)) < 0)
	{
		perror("write(5)");
		return -1;
	}
	return 0;
}

 int RecordStream::check_h26x_info(enum iav_stream_type stream_type, int stream_id)
{
	struct iav_h26x_cfg h26x;

	memset(&h26x, 0, sizeof(h26x));
	h26x.id = stream_id;
	if (stream_type == IAV_STREAM_TYPE_H264)
	{
		AM_IOCTL(fd_iav, IAV_IOC_GET_H264_CONFIG, &h26x);
	}
	else
	{
		AM_IOCTL(fd_iav, IAV_IOC_GET_H265_CONFIG, &h26x);
	}

	if (h26x.gop_structure < IAV_GOP_SVCT_2 ||
		h26x.gop_structure > IAV_GOP_FAST_SEEK_2_REF)
	{
		fprintf(stderr, "Invalid gop structure %d for AVC or HEVC!\n", h26x.gop_structure);
		return -1;
	}
	stream_files.gop_structure = h26x.gop_structure;

	if (split_fast_seek_flag && !h26x.fast_seek_intvl)
	{
		fprintf(stderr, "Invalid fast seek intvl value %d.\n", h26x.fast_seek_intvl);
		return -1;
	}
	stream_files.fast_seek_intvl = h26x.fast_seek_intvl;

	return 0;
}

// check session and update file handle for write when needed
 int RecordStream::check_session_file_handle(struct iav_framedesc *framedesc, int new_session)
{
	char write_file_name[480] = {0};
	char time_str[128];
	char file_type[8];
	size_t len = 0;
	int i;
	int stream_id = framedesc->id;
	char stream_name;

	enum iav_stream_type stream_type = framedesc->stream_type;

	if (new_session)
	{
		snprintf(file_type, sizeof(file_type), "%s", (stream_type == IAV_STREAM_TYPE_MJPEG) ? "mjpeg" : ((stream_type == IAV_STREAM_TYPE_H264) ? "h264" : (((stream_type == IAV_STREAM_TYPE_H265) ? "h265" : "unknown"))));
		// close old session if needed
		if (stream_files.fd > 0)
		{
			close(stream_files.fd);
			stream_files.fd = -1;
		}
		// character based stream name
		if (split_svct_layer_flag)
		{
			for (i = 0; i < MAX_SVCT_LAYERS; ++i)
			{
				if (stream_files.fd_svct[i] > 0)
				{
					stream_files.fd_svct[i] = -1;
				}
			}
			stream_files.gop_structure = 0;
			stream_files.fast_seek_intvl = 0;
		}
		if (split_fast_seek_flag)
		{
			if (stream_files.fd_fast_seek > 0)
			{
				stream_files.fd_fast_seek = -1;
			}
		}

		stream_name = 'A' + stream_id;

		get_time_string(time_str, sizeof(time_str));
		if (remove_time_string_flag)
		{
			memset(time_str, 0, sizeof(time_str));
		}

		len = strlen(filename);
		if (len >= sizeof(write_file_name))
		{
			fprintf(stderr, "filename is too long (%lu >= %lu) \n", len, sizeof(write_file_name));
			return -1;
		}
		memcpy(write_file_name, filename, len);

		if (md5_idr_number <= 0)
		{
			snprintf(write_file_name + len, sizeof(write_file_name) - len, "_%c_%s_%x.%s",
					 stream_name, time_str, framedesc->session_id,
					 (stream_type == IAV_STREAM_TYPE_MJPEG) ? "mjpeg" : ((stream_type == IAV_STREAM_TYPE_H264) ? "h264" : (((stream_type == IAV_STREAM_TYPE_H265) ? "h265" : "unknown"))));
		}
		if (stream_files.fd < 0)
		{
			if ((stream_files.fd = open(filename, O_CREAT | O_RDWR | O_APPEND)) < 0)
			{
				fprintf(stderr, "create file for write failed %s \n", filename);
				return -1;
			}
			else
			{
				if (!nofile_flag)
				{
					printf("\nnew session file name [%s], fd [%d] \n", filename,  stream_files.fd);
				}
			}
		}
		if (split_svct_layer_flag || split_fast_seek_flag)
		{
			check_h26x_info(stream_type, stream_id);
		}

		if (split_svct_layer_flag)
		{
			if (stream_files.gop_structure > 1 && stream_files.gop_structure <= 4)
			{
				for (i = 0; i < stream_files.gop_structure; ++i)
				{
					snprintf(filename, sizeof(filename), "%s.svct_%d.%s", write_file_name, i, file_type);
					
					stream_files.fd_svct[i] = open("", O_RDWR | O_APPEND); /// amba_transfer_open(filename, method, (port + i + SVCT_PORT_OFFSET));
					if (stream_files.fd_svct[i] < 0)
					{
						fprintf(stderr, "create file for write SVCT layers failed %s.\n",filename);
						return -1;
					}
				}
			}
		}
		if (split_fast_seek_flag)
		{
			if (stream_files.gop_structure > 1 && stream_files.fast_seek_intvl > 0)
			{
				snprintf(filename, sizeof(filename), "%s.fast_seek.%s", write_file_name, file_type);
				
				stream_files.fd_fast_seek = open("", O_RDWR | O_APPEND); /// amba_transfer_open(filename, method, (port + FAST_SEEK_PORT_OFFSET));
				if (stream_files.fd_fast_seek < 0)
				{
					fprintf(stderr, "create file for write fast seek failed %s.\n", filename);
					return -1;
				}
			}
		}

		if (frame_info_flag)
		{
			strncat(write_file_name, ".info", sizeof(write_file_name) - strlen(write_file_name) - 1);
			if ((stream_files.fd_info = open("", O_RDWR | O_APPEND)) < 0) ///	amba_transfer_open(write_file_name, method, port)) < 0)
			{
				fprintf(stderr, "create file for frame info  failed %s \n", write_file_name);
				return -1;
			}
			if (write_frame_info_header() < 0)
			{
				fprintf(stderr, "write h264 header info failed %s \n", write_file_name);
				return -1;
			}
		}
	}
	return 0;
}

 int RecordStream:: update_files_data(struct iav_framedesc *framedesc, int new_session)
{
	if (new_session)
	{
		stream_files.session_id = framedesc->session_id;
		stream_files.total_bytes = 0;
	}

	stream_files.total_bytes += framedesc->size;

	return 0;
}

 int RecordStream::write_svct_file(unsigned char *in, int len, int fd)
{
	if (write(fd, in, len) < 0)
	{
		perror("Failed to write stream into SVCT file.\n");
		return -1;
	}
	return 0;
}

int RecordStream::write_fast_seek_file(unsigned char *in, int len, int fd)
{
	if (write(fd, in, len) < 0)
	{
		perror("Failed to write stream into fast seek file.\n");
		return -1;
	}
	return 0;
}

 int RecordStream::identify_nal_ref_idc(unsigned char *in, int in_len)
{
	const int header_magic_number = 0x000001;
	unsigned int header_mn = 0;
	unsigned char nalu, nal_ref_idc = -1;
	int i = 0;

	do
	{
		header_mn = (in[i] << 16 | in[i + 1] << 8 | in[i + 2]);
		if (header_mn == header_magic_number)
		{
			i += 3;
			nalu = in[i] & 0x1F;
			if ((nalu == NT_IDR) || nalu == NT_NON_IDR)
			{
				nal_ref_idc = ((in[i] >> 5) & 0x3);
				break;
			}
		}
		++i;
	} while (i < in_len);

	return nal_ref_idc;
}

 int RecordStream::identify_nuh_temporal_id_plus1(unsigned char *in, int in_len)
{
	const int header_magic_number = 0x000001;
	unsigned int header_mn = 0;
	unsigned char nalu, nuh_temporal_id_plus1 = -1;
	int i = 0;

	do
	{
		header_mn = (in[i] << 16 | in[i + 1] << 8 | in[i + 2]);
		if (header_mn == header_magic_number)
		{
			i += 3;
			nalu = (in[i] & 0x7F) >> 1;
			if ((nalu == IDR_W_RADL) || (nalu == TRAIL_R) || nalu == TSA_N)
			{
				nuh_temporal_id_plus1 = (in[i + 1] & 0x7);
				break;
			}
		}
		++i;
	} while (i < in_len);

	return nuh_temporal_id_plus1;
}

 int RecordStream::get_svct_layer(enum iav_stream_type stream_type, int stream_id, unsigned char *in,
						  int in_len, int *ret_layer)
{

	int with_fast_seek = ((stream_files.fast_seek_intvl > 0) ? 1 : 0);
	int gop = stream_files.gop_structure;
	int rval = 0;
	int layer, index, is_h264;

	if (!ret_layer)
	{
		fprintf(stderr, "Invalid return layer pointer!\n");
		return -1;
	}

	is_h264 = ((stream_type == IAV_STREAM_TYPE_H264) ? 1 : 0);

	if (is_h264)
	{
		index = identify_nal_ref_idc(in, in_len);
	}
	else
	{
		index = identify_nuh_temporal_id_plus1(in, in_len);
	}

	switch (gop)
	{
	case IAV_GOP_SVCT_4:
		if (is_h264)
		{
			if (index < 0 || index > 3)
			{
				rval = -1;
				fprintf(stderr, "Invalid index %d\n", index);
				break;
			}
			if (with_fast_seek)
			{
				rval = -1;
				fprintf(stderr, "Cannot support fast seek for SVCT\n");
				break;
			}
			layer = abs(3 - index);
		}
		else
		{
			if (index < 1 || index > 4)
			{
				rval = -1;
				fprintf(stderr, "Invalid index %d\n", index);
				break;
			}
			if (with_fast_seek)
			{
				rval = -1;
				fprintf(stderr, "Cannot support fast seek for SVCT\n");
				break;
			}
			layer = index - 1;
		}
		break;
	case IAV_GOP_SVCT_3:
		if (is_h264)
		{
			if (!with_fast_seek)
			{
				switch (index)
				{
				case 3:
					layer = 0;
					break;
				case 2:
					layer = 1;
					break;
				case 0:
					layer = 2;
					break;
				default:
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
			}
			else
			{
				switch (index)
				{
				case 2:
				case 3:
					layer = 0;
					break;
				case 1:
					layer = 1;
					break;
				case 0:
					layer = 2;
					break;
				default:
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
			}
		}
		else
		{
			if (!with_fast_seek)
			{
				if (index < 1 || index > 3)
				{
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
				layer = index - 1;
			}
			else
			{
				if (index < 0 || index > 4)
				{
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
				if (index == 1)
				{
					layer = 0;
				}
				else
				{
					layer = index - 2;
				}
			}
		}
		break;
	case IAV_GOP_SVCT_2:
		if (is_h264)
		{
			if (!with_fast_seek)
			{
				switch (index)
				{
				case 3:
					layer = 0;
					break;
				case 0:
					layer = 1;
					break;
				default:
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
			}
			else
			{
				switch (index)
				{
				case 2:
				case 3:
					layer = 0;
					break;
				case 0:
					layer = 1;
				default:
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
			}
		}
		else
		{
			if (!with_fast_seek)
			{
				if ((index < 1) && (index > 2))
				{
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
				layer = index - 1;
			}
			else
			{
				switch (index)
				{
				case 1:
				case 2:
					layer = 0;
					break;
				case 3:
					layer = 1;
					break;
				default:
					rval = -1;
					fprintf(stderr, "Invalid index %d\n", index);
					break;
				}
			}
		}
		break;
	default:
		rval = -1;
		fprintf(stderr, "Invalid SVCT gop structure %d, cannot be larger than 4.\n", gop);
		break;
	}

	*ret_layer = layer;

	return rval;
}

 int RecordStream::write_svct_files(enum iav_stream_type stream_type, /*int transfer_method,*/
							int stream_id, unsigned char *in, int in_len)
{
	int gop = stream_files.gop_structure;
	int layer = -1, rval = 0;

	if (get_svct_layer(stream_type, stream_id, in, in_len, &layer) < 0)
	{
		fprintf(stderr, "get svct layer failed!\n");
		return -1;
	}
	switch (gop)
	{
	case IAV_GOP_SVCT_4:
		switch (layer)
		{
		case 0:
			write_svct_file(in, in_len,
							stream_files.fd_svct[3]);
			/* Fall through to write this frame into other layers */
		case 1:
			write_svct_file(in, in_len,
							stream_files.fd_svct[2]);
			/* Fall through to write this frame into other layers */
		case 2:
			write_svct_file(in, in_len,
							stream_files.fd_svct[1]);
			/* Fall through to write this frame into other layers */
		case 3:
			write_svct_file(in, in_len,
							stream_files.fd_svct[0]);
			break;
		default:
			rval = -1;
			fprintf(stderr, "Incorrect SVCT layer [%d] from bitstream!\n", layer);
			break;
		}
		break;
	case IAV_GOP_SVCT_3:
		switch (layer)
		{
		case 0:
			write_svct_file(in, in_len,
							stream_files.fd_svct[2]);
			/* Fall through to write this frame into other layers */
		case 1:
			write_svct_file(in, in_len,
							stream_files.fd_svct[1]);
			/* Fall through to write this frame into other layers */
		case 2:
			write_svct_file(in, in_len,
							stream_files.fd_svct[0]);
			break;
		default:
			rval = -1;
			fprintf(stderr, "Incorrect SVCT layer [%d] from bitstream!\n", layer);
			break;
		}
		break;
	case IAV_GOP_SVCT_2:
		switch (layer)
		{
		case 0:
			write_svct_file(in, in_len,
							stream_files.fd_svct[1]);
			/* Fall through to write this frame into other layers */
		case 1:
			write_svct_file(in, in_len,
							stream_files.fd_svct[0]);
			break;
		default:
			rval = -1;
			fprintf(stderr, "Incorrect SVCT layer [%d] from bitstream!\n", layer);
			break;
		}
		break;
	}

	if ((rval >= 0) && verbose_mode)
	{
		printf("Save SVCT layer [%d] into file.\n", layer);
	}

	return rval;
}

int RecordStream::write_fast_seek_files(enum iav_stream_type stream_type,
								 int stream_id, unsigned char *in, int in_len)
{
	int fd = stream_files.fd_fast_seek;
	int gop = stream_files.gop_structure;
	int nal_ref_idc, nuh_temporal_id_plus1;
	int rval = 0;

	if (stream_type == IAV_STREAM_TYPE_H264)
	{
		nal_ref_idc = identify_nal_ref_idc(in, in_len);
		switch (gop)
		{
		case IAV_GOP_SVCT_2:
		case IAV_GOP_SVCT_3:
		case IAV_GOP_SVCT_4:
			if (nal_ref_idc == 3)
			{
				write_fast_seek_file(in, in_len, fd);
			}
			break;
		case IAV_GOP_FAST_SEEK:
		case IAV_GOP_FAST_SEEK_2_REF:
			if (nal_ref_idc == 2 || nal_ref_idc == 3)
			{
				write_fast_seek_file(in, in_len, fd);
			}
			break;
		default:
			rval = -1;
			fprintf(stderr, "Invalid fast seek gop structure.\n");
			break;
		}
	}
	else if (stream_type == IAV_STREAM_TYPE_H265)
	{
		nuh_temporal_id_plus1 = identify_nuh_temporal_id_plus1(in, in_len);
		if (nuh_temporal_id_plus1 == 1)
		{
			write_fast_seek_file(in, in_len, fd);
		}
	}
	else
	{
		rval = -1;
	}

	if ((rval >= 0) && verbose_mode)
	{
		printf("Save fast seek frames into files.\n");
	}

	return rval;
}

 int RecordStream::write_video_file(struct iav_framedesc *framedesc, int new_frame)
{
	u32 pic_size = framedesc->size;
	int fd = stream_files.fd;
	int stream_id = framedesc->id;
	enum iav_stream_type stream_type = framedesc->stream_type;

	if (md5_idr_number > 0)
	{
		// printf("write frame %d\n",framedesc->frame_num);
		if (write(fd, bsb_mem + framedesc->data_addr_offset, pic_size /*,	stream_transfer[stream_id].method*/) < 0)
		{
			perror("Failed to write specify streams into file!\n");
			return -1;
		}
		if (new_frame)
			md5_idr_number = md5_idr_number - 1;
		return 0;
	}
	
	if (write(fd, bsb_mem + framedesc->data_addr_offset, pic_size /*,	stream_transfer[stream_id].method */) < 0)
	{
		perror("Failed to write streams into file!\n");
		return -1;
	}
	if (split_svct_layer_flag && (stream_files.gop_structure > 1) && (stream_files.gop_structure <= 4))
	{
		if (write_svct_files(stream_type, /*method,*/ stream_id,
							 (unsigned char *)bsb_mem + framedesc->data_addr_offset, pic_size) < 0)
		{
			perror("Failed to split and write SVCT layers into files!\n");
			return -1;
		}
	}
	if (split_fast_seek_flag && (stream_files.gop_structure > 1))
	{
		if (write_fast_seek_files(stream_type, /*method,*/ stream_id,
								  (unsigned char *)bsb_mem + framedesc->data_addr_offset, pic_size) < 0)
		{
			perror("Failed to split and write fast seek frames into files!\n");
			return -1;
		}
	}

	return 0;
}

 int RecordStream::flush_frame_desc(void)
{
	struct iav_queryinfo query_info;
	struct iav_bsb_stats_info *bsb_stats;
	int rval = 0;

	memset(&query_info, 0, sizeof(query_info));
	query_info.qid = IAV_INFO_BSB_STATS;
	bsb_stats = &query_info.arg.bsb_stats;
	AM_IOCTL(fd_iav, IAV_IOC_QUERY_INFO, &query_info);

	if ((bsb_stats->frame_drop_cnt != 0) && (bsb_stats->bsb_mode == IAV_DUAL_BSB))
	{
		printf("In IAV DUAL BSB mode, frame drop count[%d], frame lock queue will be flushed \n",
			   bsb_stats->frame_drop_cnt);
		AM_IOCTL(fd_iav, IAV_IOC_FLUSH_FRAMEDESC, NULL);
	}

	return rval;
}

int RecordStream::release_frame_desc(struct iav_framedesc *frame_desc)
{
	AM_IOCTL(fd_iav, IAV_IOC_RELEASE_FRAMEDESC, frame_desc);
	return 0;
}

 int RecordStream::show_bsb_stats(void)
{
	struct iav_queryinfo query_info;
	struct iav_bsb_stats_info *bsb_stats;
	int rval = 0;

	memset(&query_info, 0, sizeof(query_info));
	query_info.qid = IAV_INFO_BSB_STATS;
	bsb_stats = &query_info.arg.bsb_stats;
	AM_IOCTL(fd_iav, IAV_IOC_QUERY_INFO, &query_info);

	if (bsb_stats->bsb_mode == IAV_DUAL_BSB)
	{
		printf("BSB stats: free_kbytes = %d, bsb_mode = %d, frame_drop_cnt = %d, frame_locked_cnt = %d \n",
			   bsb_stats->free_kbytes, bsb_stats->bsb_mode, bsb_stats->frame_drop_cnt, bsb_stats->frame_locked_cnt);
	}
	else
	{
		printf("BSB stats: free_kbytes = %d, bsb_mode = %d, frame_drop_cnt = %d \n",
			   bsb_stats->free_kbytes, bsb_stats->bsb_mode, bsb_stats->frame_drop_cnt);
	}

	return rval;
}

int RecordStream::write_stream(u64 *total_frames, u64 *total_bytes)
{
	struct iav_queryinfo query_info;
	struct iav_stream_info *stream_info;
	struct iav_querydesc query_desc;
	struct iav_framedesc *frame_desc;
	static int init_flag = 0;
	static int end_of_stream; ///[MAX_ENCODE_STREAM_NUM];
	int new_session;		  // 0:  old session  1: new session
	int new_frame;
	int stream_id;
	struct timeval curr;
	int i, stream_end_num;
	
	u32 active_stream_mask = 0, active_stream_num = 0;
	u8 stream_state_changed = 0;

	if (init_flag == 0)
	{
		
		end_of_stream = 1;
	
		init_flag = 1;
	}

	
	{
		query_info.qid = IAV_INFO_STREAM;
		stream_info = &query_info.arg.stream;
		stream_info->id = ENCODEN_VIDEO_STREAM_ID; /// i;
		AM_IOCTL(fd_iav, IAV_IOC_QUERY_INFO, &query_info);

		if (stream_info->state == IAV_STREAM_STATE_ENCODING)
		{
			++active_stream_num;
			
			end_of_stream = 0;
		}

		
	}
	// check if the encode state changed
	if (stream_mask != active_stream_mask)
	{
		stream_state_changed = 1;
		stream_mask = active_stream_mask; // update the old status.
	}

	// There is no encoding stream, skip to next turn
	//	if (stream_end_num == MAX_ENCODE_STREAM_NUM)
	//		return -1;

	memset(&query_desc, 0, sizeof(query_desc));
	frame_desc = &query_desc.arg.frame;
	query_desc.qid = IAV_DESC_FRAME;
	frame_desc->id = -1;
	if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0)
	{
		if (errno != EAGAIN)
		{
			perror("IAV_IOC_QUERY_DESC");
		}
		return -1;
	}

	// check if it's new record session, since file name and recording control are based on session,
	// session id and change are important data
	if (frame_desc->size >= get_disk_space_free())
	{
		fprintf(stderr, "No enough free space \n");
		return -2;
	}
	stream_id = frame_desc->id;
	new_session = is_new_session(frame_desc);
	new_frame = is_new_frame(frame_desc);
	
	// check if it's a stream end null frame indicator
	if (frame_desc->stream_end)
	{
		end_of_stream = 1;
		deinit_stream_files();
		goto write_stream_exit;
	}

	if (update_files_data(frame_desc, new_session) < 0)  /// update stream_files.session_id 
	{
		fprintf(stderr, "update files data failed \n");
		return -2;
	}

	// check and update session file handle
	if (check_session_file_handle(frame_desc, new_session) < 0)  /// update stream_files.fd
	{
		fprintf(stderr, "check session file handle failed \n");
		return -3;
	}

	if (frame_info_flag)
	{
		if (write_frame_info(frame_desc) < 0)
		{
			fprintf(stderr, "write video frame info failed for stream %d, session id = %d.\n",
					stream_id, frame_desc->session_id);
			return -5;
		}
	}

	// write file if file is still opened
	if (write_video_file(frame_desc, new_frame) < 0)
	{
		fprintf(stderr, "write video file failed for stream %d, session id = %d \n",
				stream_id, frame_desc->session_id);
		return -4;
	}
////printf("new_session:%d----fd_iav:%d \n",new_session,fd_iav);
	release_frame_desc(frame_desc);
	if (show_bsb_stats_flag)
	{
		show_bsb_stats();
	}

	// update global statistics
	if (total_frames && new_frame)
		*total_frames = (*total_frames) + 1;
	if (total_bytes)
		*total_bytes = (*total_bytes) + frame_desc->size;

write_stream_exit:
	gettimeofday(&curr, NULL);
	if (((curr.tv_sec - pre.tv_sec) * 1000 + (curr.tv_usec - pre.tv_usec) / 1000) >= TEN_MINUTE_TO_MISECOND)
	{
		pre.tv_sec = curr.tv_sec;
		pre.tv_usec = curr.tv_usec;
		if(stream_files.fd > 0)close(stream_files.fd);
		///stream_files.fd = -1;
		memset(filename, 0x00, sizeof(filename));
		record_file_index++;
		sprintf(filename, "%s_%d.h264", default_filename, record_file_index);
		if ((stream_files.fd = open(filename, O_CREAT | O_RDWR | O_APPEND)) < 0)
		{
			fprintf(stderr, "create file for write failed %s \n", filename);
        	return -1;
		}
	
	}
	return 0;
}

 int RecordStream::show_waiting(void)
{
#define DOT_MAX_COUNT 10
	static int dot_count = DOT_MAX_COUNT;
	int i;

	if (dot_count < DOT_MAX_COUNT)
	{
		fprintf(stdout, "."); // print a dot to indicate it's alive
		dot_count++;
	}
	else
	{
		fprintf(stdout, "\r");
		for (i = 0; i < 80; i++)
			fprintf(stdout, " ");
		fprintf(stdout, "\r");
		dot_count = 0;
	}

	fflush(stdout);
	return 0;
}

int RecordStream::map_bsb(void)
{
	struct iav_querymem query_mem = {(enum iav_mem_id)0};
	struct iav_mem_part_info *part_info = NULL;

	query_mem.mid = IAV_MEM_PARTITION;
	part_info = &query_mem.arg.partition;
	part_info->pid = IAV_PART_BSB; /// BSB : bit stream partition
	if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0)
	{
		perror("IAV_IOC_QUERY_MEMBLOCK");
		return -1;
	}

	if (part_info->mem.length == 0)
	{
		fprintf(stderr, "IAV_PART_BSB is not allocated.\n");
		return -1;
	}

	bsb_size = part_info->mem.length;
	bsb_mem = (u8 *)mmap(NULL, bsb_size * 2, PROT_READ, MAP_SHARED, fd_iav, part_info->mem.addr);
	if (bsb_mem == MAP_FAILED)
	{
		perror("mmap (%d) failed: %s\n");
		return -1;
	}

	printf("bsb_mem = %p, size = 0x%x\n", bsb_mem, bsb_size);

	return 0;
}
long long RecordStream::get_disk_space_free(void)
{
	struct statfs spaceInfo;
	statfs(DISK_DIRECTORY, &spaceInfo);
	unsigned long long totalBlocks = spaceInfo.f_bsize;
	unsigned long long freeSpace = spaceInfo.f_bfree * totalBlocks;
	return freeSpace;
	//*free = freeDisk;
	// return 0;
}
int RecordStream::init_data(void)
{
	int ret = 0;

	if ((fd_iav = open("/dev/iav", O_RDWR, 0)) < 0)
	{
		perror("/dev/iav");
		return -1;
	}
	if (get_disk_space_free() <= 0)
	{
		perror("No space free");
		return -1;
	}

	if (map_bsb() < 0)
	{
		fprintf(stderr, "map bsb failed\n");
		ret = -1;
		return -1;
	}

	init_stream_files();
	flush_frame_desc();
	gettimeofday(&pre, NULL);

	return ret;
}

int RecordStream::capture_encoded_video(int run_write_video_file)
{
	int rval;
	// open file handles to write to
	u64 total_frames;
	u64 total_bytes;
	total_frames = 0;
	total_bytes = 0;

#ifdef ENABLE_RT_SCHED
	{
		struct sched_param param;
		param.sched_priority = 99;
		if (sched_setscheduler(0, SCHED_FIFO, &param) < 0)
			perror("sched_setscheduler");
	}
#endif
	flush_frame_desc();
	gettimeofday(&pre, NULL);
	while (run_write_video_file)
	{
		if ((rval = write_stream(&total_frames, &total_bytes)) < 0)
		{
			if (rval == -1)
			{
				usleep(100 * 1000);
				show_waiting();
			}
			else
			{
				fprintf(stderr, "write_stream err code %d \n", rval);
				break;
			}
			continue;
		}
		if (md5_idr_number == 0)
		{
			md5_idr_number = -1;
			/// statistics_run = 0;
			break;
		}
	}

	printf("stop encoded stream capture\n");

	printf("total_frames = %lld\n", total_frames);
	printf("total_bytes = %lld\n", total_bytes);

	return 0;
}

int RecordStream::deinit_data()
{
	close_stream_files();
	
	if (fd_iav)
		close(fd_iav);
	fd_iav = -1;
	return 0;
}
#ifdef __cplusplus
}
#endif