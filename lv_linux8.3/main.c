#include "lvgl/lvgl.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl/examples/lv_examples.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include "UI/ui.h"  // 包含UI相关的头文件

#define DISP_BUF_SIZE (1024 * 1024)





int main(void)
{
    /* LVGL初始化 */
    lv_init();

    /* 帧缓冲驱动初始化（Linux显示） */
    fbdev_init();

     /* 显示缓冲区初始化 */
    static lv_color_t buf[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*显示驱动注册*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 800;
    disp_drv.ver_res    = 480;
    lv_disp_drv_register(&disp_drv);

    // /*触摸驱动注册*/
    // evdev_init();
    // static lv_indev_drv_t indev_drv_1;
    // lv_indev_drv_init(&indev_drv_1); /*Basic initialization*/
    // indev_drv_1.type = LV_INDEV_TYPE_POINTER;

    // /*This function will be called periodically (by the library) to get the mouse position and state*/
    // indev_drv_1.read_cb = evdev_read;
    // lv_indev_t *mouse_indev = lv_indev_drv_register(&indev_drv_1);


    // /*鼠标光标设置*/
    // LV_IMG_DECLARE(mouse_cursor_icon)
    // lv_obj_t * cursor_obj = lv_img_create(lv_scr_act()); /*Create an image object for the cursor */
    // lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
    // lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/

    /* 初始化命令队列和互斥锁 */
    queue_init(&cmd_queue);
    pthread_mutex_init(&cmd_mutex, NULL);

    /* 创建终端输入线程 */
    pthread_t tid;
    if (pthread_create(&tid, NULL, terminal_input_thread, NULL) != 0) {
        perror("[ERROR] 创建终端线程失败");
        return -1;
    }

    ui_init();  // 初始化UI

    /* 创建定时器：1秒更新时间 */
    lv_timer_create(update_time, 1000, NULL);

    /* 创建定时器：50ms处理命令队列（保证UI响应流畅） */
    lv_timer_create(process_cmd_queue, 50, NULL);

    /*****************************************************************************
     * 4.8 主循环（程序核心，不能退出）
     * 说明：周期性调用LVGL定时器处理函数，让UI响应事件、更新动画
     * 注意：usleep的时间不能太长（建议5~10ms），否则UI会卡顿；也不能太短，否则占用CPU过高
     ****************************************************************************/
    while(1) {
        lv_timer_handler();
        usleep(5000);
    }

     /* 资源清理（实际不会执行到） */
    pthread_mutex_destroy(&cmd_mutex);
    return 0;

    return 0;
}

/* LVGL时钟回调（必须实现） */
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
