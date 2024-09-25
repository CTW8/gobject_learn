#include "test1.h"

// 定义一个简单的 GObject 对象
G_DEFINE_TYPE(MyObject, my_object, G_TYPE_OBJECT)

// 初始化实例
static void my_object_init(MyObject *self) {
    self->value = 42;
}

// 初始化类
static void my_object_class_init(MyObjectClass *klass) {
    // 类初始化代码
}