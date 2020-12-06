#include <fstream>
#include <iostream>
using namespace std;

extern "C"
{
  #include <libavcodec/codec_id.h>
  #include <bits/stdint-uintn.h>
  #include <libavcodec/codec.h>
  #include <libavcodec/codec_par.h>
  #include <libavutil/frame.h>
  #include <libavcodec/packet.h>
  #include <libavutil/mem.h>
  #include <libavutil/pixfmt.h>
  #include <libavcodec/avcodec.h>
  #include <libavutil/imgutils.h>
  #include <libswscale/swscale.h>
  #include <libavformat/avformat.h>
  #include <libavutil/common.h>
}

/**
 * @brief Opens an image file and decodes the file contents into an AVFrame object.
 * 
 * @param filename Path to the target image file.
 * @param codec_id (Optional out param) Codec id found in the function.
 * @return AVFrame* Frame containing the decoded image data.
 */
AVFrame * OpenFile (const char *filename, int *codec_id = NULL)
{
  AVFormatContext *format_ctx = NULL;

  int error_code;
  error_code = avformat_open_input(&format_ctx, filename, NULL, NULL);
  if (error_code != 0)
  {
    cout << "Could not open " << filename << endl;
    return NULL;
  }

  error_code = avformat_find_stream_info(format_ctx, NULL);
  if (error_code != 0)
  {
    cout << "Could not find stream info for " << filename << endl;
    return NULL;
  }

  AVCodecParameters *codec_par = format_ctx->streams[0]->codecpar;
  *codec_id = codec_par->codec_id;
  AVCodec *codec = avcodec_find_decoder(codec_par->codec_id);
  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);

  if (codec == NULL)
  {
    cout << "Could not find codec info for " << filename << endl;
    return NULL;
  }

  error_code = avcodec_open2(codec_ctx, codec, NULL);
  if (error_code != 0)
  {
    cout << "Could not open codec for " << filename << endl;
    return NULL;
  }

  AVFrame *frame;
  AVPacket packet;

  frame = av_frame_alloc();
  if (frame == NULL)
  {
    cout << "Could not allocate frame" << endl;
    return NULL;
  }

  error_code = av_read_frame(format_ctx, &packet);
  if (error_code != 0)
  {
    cout << "Could not read frame" << endl;
    return NULL;
  }

  error_code = avcodec_send_packet(codec_ctx, &packet);
  if (error_code != 0)
  {
    cout << "Could not send packet" << endl;
    return NULL;
  }

  error_code = avcodec_receive_frame(codec_ctx, frame);
  if (error_code != 0)
  {
    cout << "Could not receive frame" << endl;
    return NULL;
  }

  return frame;
}

/**
 * @brief Writes a frame objects out into an image file.
 * 
 * @param filename Path to output image file.
 * @param frame Frame containing the data being written to the file.
 * @param codec_id Specifies which codec to use to encode the image data.
 */
void WriteFile (const char *filename, AVFrame *frame, AVCodecID codec_id)
{
  AVCodec *out_codec = avcodec_find_encoder((AVCodecID)codec_id);
  if (out_codec == NULL)
  {
    cout << "Failed to find decoder" << endl;
    return;
  }

  AVCodecContext *out_ctx = avcodec_alloc_context3(out_codec);
  if (out_ctx == NULL)
  {
    cout << "Failed to alloc context" << endl;
    return;
  }

  int error_code;
  out_ctx->pix_fmt = out_ctx->pix_fmt;
  out_ctx->width = frame->width;
  out_ctx->height = frame->height;
  out_ctx->time_base = {1, 1};

  error_code = avcodec_open2(out_ctx, out_codec, NULL);
  if (error_code < 0)
  {
    cout << "Failed to open codec" << endl;
    return;
  }

  AVPacket packet;
  packet.size = 0;
  packet.data = NULL;
  av_init_packet(&packet);

  error_code = avcodec_send_frame(out_ctx, frame); // Send frame data to the encoder
  if (error_code < 0)
  {
    cout << "Error encoding image, error: " << error_code << endl;
    return;
  } 
  cout << "Encoded" << endl;

  avcodec_receive_packet(out_ctx, &packet);

  ofstream out_file("inverted.png");
  out_file.write((const char *) packet.data, packet.size);
  out_file.close();
}

/**
 * @brief Converts (x,y) coordinates of a pixel into their linear position in a frame's data array.
 * 
 * @param x X-coordinate of target pixel.
 * @param y Y-coordinate of target pixel.
 * @param linesize Size of a horizontal row of pixels in the frame.
 * @param pix_bytes Number of bytes a pixel is stored in (4 for rgba, 3 for rgb).
 * @return int Linear index of target pixel.
 */
int LinearizePixelCoords (int x, int y, int linesize, int pix_bytes)
{
  return y*linesize + x*pix_bytes;
}

int main ()
{
  int codec_id;
  AVFrame *frame = OpenFile("traffic_map_2020_12_01_03_40.png", &codec_id);
  if (frame == NULL)
  {
    cout << "Failed" << endl;
  }

  // Iterates through each pixel in the frame and inverts their colors
  int x, y, b;
  for (x = 0; x < frame->width; x++)
    for (y = 0; y < frame->height; y++)
      for (b = 0; b < 3; b++)
        frame->data[0][LinearizePixelCoords(x, y, frame->linesize[0], 4) + b] = 255 - frame->data[0][LinearizePixelCoords(x, y, frame->linesize[0], 4) + b];

  WriteFile("inverted.png", frame, (AVCodecID)codec_id);

  return 0;
}