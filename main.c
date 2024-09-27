#include <stdio.h>
#include "test1.h"
#include "mp4_demux.h"
#include "media_player.h"
#define MP4_FILE "/Users/lizhen/Downloads/The_Winner.mp4"

// void test_player1(){
//         const char *filename = MP4_FILE;
//     DemuxMp4 *demuxer = g_object_new(TYPE_DEMUX_MP4, NULL);

//     // 打开文件
//     demux_mp4_open(demuxer, filename);
//     // 获取文件信息
//     DemuxMp4FileInfo info;
//     demux_mp4_get_file_info(demuxer, &info);
//     printf("Video Track Count: %d\n", info.video_track_count);
//     printf("Audio Track Count: %d\n", info.audio_track_count);
//     printf("Duration: %" PRId64 "\n", info.duration);

//     // 打印视频size、帧率和码率
//     if (info.video_track_count > 0) {
//         printf("Video Width: %d\n", info.video_width);
//         printf("Video Height: %d\n", info.video_height);
//         printf("Video Frame Rate: %.2f\n", info.video_frame_rate);
//         printf("Video Bit Rate: %" PRId64 "\n", info.video_bit_rate);
//     }

//     // 打印音频的sample rate、channel和sample format
//     if (info.audio_track_count > 0) {
//         printf("Audio Sample Rate: %d\n", info.audio_sample_rate);
//         printf("Audio Channels: %d\n", info.audio_channels);
//         printf("Audio Sample Format: %s\n", info.audio_sample_format);
//     }

//     // 读取并打印每个packet的pts、dts以及packet类型
//     AVPacket *packet;
//     while ((packet = demux_mp4_read(demuxer)) != NULL) {
//         DemuxMp4FileInfo info;
//         demux_mp4_get_file_info(demuxer, &info);
//         const char *packet_type = "Unknown";
//         if (packet->stream_index == info.video_index) {
//             packet_type = "Video";
//         } else if (packet->stream_index == info.audio_index) {
//             packet_type = "Audio";
//         }
//         printf("PTS: %" PRId64 ", DTS: %" PRId64 ", Packet Type: %s, Index: %d\n", packet->pts, packet->dts, packet_type, packet->stream_index);
//         av_packet_unref(packet);
//         av_packet_free(&packet);
//     }

//     // 关闭文件
//     demux_mp4_close(demuxer);
//     g_object_unref(demuxer);
// }

void test_player2(){
    const char *filename = MP4_FILE;
    MediaPlayer *player = g_object_new(TYPE_MEDIA_PLAYER, NULL);

    media_player_initialize(player, filename);
    media_player_play(player);

    // 模拟播放一段时间
    SDL_Delay(10000);

    media_player_pause(player);
    media_player_seek(player, 5000);
    media_player_play(player);

    // 模拟播放一段时间
    SDL_Delay(10000);

    media_player_stop(player);
    g_object_unref(player);
}

int main(int argc, char** argv) {
    // if (argc < 2) {
    //     fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
    //     return -1;
    // }

    test_player2();

    return 0;
}

