#include <stdio.h>
#include "test1.h"

int main(int argc, char** argv) {
    // 初始化 GObject 类型系统
    g_type_init();

    // 创建 MyObject 实例
    MyObject *obj = g_object_new(MY_TYPE_OBJECT, NULL);

    // 使用 MyObject 实例
    printf("MyObject value: %d\n", obj->value);

    // 释放 MyObject 实例
    g_object_unref(obj);

    return 0;
}

