#ifndef MY_OBJECT_H
#define MY_OBJECT_H

#include <glib-object.h>

G_BEGIN_DECLS
#define MY_TYPE_OBJECT (my_object_get_type())

// GObject 类型宏定义
G_DECLARE_FINAL_TYPE(MyObject, my_object, MY, OBJECT, GObject)

// 定义 MyObject 结构体，继承自 GObject
struct _MyObject {
    GObject parent_instance; // 父类实例
    int value; // 自定义的成员变量
};

G_END_DECLS

#endif // MY_OBJECT_H