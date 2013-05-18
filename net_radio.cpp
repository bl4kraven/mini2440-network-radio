// @author: lbzhung
// @brief:  just a network radio player and control by mouse.
//          using mplayer as backend, left button previous, right button forward and middle button pause.
// @see  linux kernel drivers/input/mousedev.c
//       http://www.computer-engineering.org/ps2mouse/
//       
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

typedef unsigned char BYTE;

#pragma pack(1)
struct imps2_data
{
    BYTE btn_left:1;
    BYTE btn_right:1;
    BYTE btn_middle:1;
    BYTE NONE:1;
    BYTE x_sign:1;  // x offset sign
    BYTE y_sign:1;  // y offset sign
    BYTE x_overflow:1; // x offset is overflow
    BYTE y_overflow:1; // y offset is overflow

    // x/y movement offset relative to its position
    signed char x;
    signed char y;
    signed char z;
};
#pragma pack()

int main(int argc, char *argv[])
{
    BYTE mousedev_imps_seq[] = { 0xf3, 200, 0xf3, 100, 0xf3, 80 };
    int mice_fd = open("/dev/input/mice", O_RDWR|O_NONBLOCK);
    if (mice_fd == -1)
    {
        fprintf(stderr, "Open mice fail");
        return 1;
    }

    // set mice mode, so can read rolling wheels
    int nRet = write(mice_fd, mousedev_imps_seq, sizeof(mousedev_imps_seq));
    if (nRet < 0)
    {
        fprintf(stderr, "set mice imps2 fail\n");
        return 1;
    }


    FILE *stream = popen("mplayer -quiet -softvol -softvol-max 300 -playlist channel.txt", "w");
    if (stream == NULL || stream < 0)
    {
        fprintf(stderr, "popen mplayer fail\n");
        return 1;
    }

    while (true)
    {
        fd_set fdset;
        //struct timeval tv;
        //tv.tv_sec = 0;
        //tv.tv_usec = 200000;

        FD_ZERO(&fdset);
        FD_SET(mice_fd, &fdset);

        int ret = select(mice_fd+1, &fdset, NULL, NULL, NULL);
        if (ret < 0)
        {
            fprintf(stderr, "select return error\n");
            return 1;
        }
        else
        {
            if (FD_ISSET(mice_fd, &fdset))
            {
                imps2_data data;
                while (true)
                {
                    int nLen = read(mice_fd, &data, sizeof(data));
                    if (nLen == 0)
                    {
                        fprintf(stderr, "End of file :)\n");
                        return 1;
                    }
                    else if (nLen == -1)
                    {
                        if (errno != EAGAIN)
                        {
                            fprintf(stderr, "read fail\n");
                            return 1;
                        }
                        break;
                    }
                    else
                    {
                        // nLen = 1 is ack
                        if (nLen == sizeof(data))
                        {
                            //printf("nLen:%d, left:%d right:%d middle:%d X:%d Y:%d Z:%d\n",
                            //        nLen, data.btn_left, data.btn_right, data.btn_middle, data.x, data.y, data.z);
                            if (data.btn_left)
                            {
                                if (data.btn_right)
                                {
                                    // OK quit
                                    close(mice_fd);

                                    fprintf(stream, "q");
                                    fflush(stream);

                                    // wait process exit
                                    pclose(stream);
                                    return 0;
                                }

                                // preivous
                                fprintf(stream, "<");
                                fflush(stream);
                            }
                            else if (data.btn_right)
                            {
                                // forword
                                fprintf(stream, ">");
                                fflush(stream);
                            }
                            else if (data.btn_middle)
                            {
                                // pause
                                fprintf(stream, "p");
                                fflush(stream);
                            }
                            else if (data.z != 0)
                            {
                                if (data.z > 0)
                                {
                                    // rolling down
                                    fprintf(stream, "9");
                                    fflush(stream);
                                }
                                else
                                {
                                    // rolling up
                                    fprintf(stream, "0");
                                    fflush(stream);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return 0;
}
