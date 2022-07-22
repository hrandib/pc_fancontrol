#ifndef MACRO_H
#define MACRO_H

#define DISABLE_OBJ_COPY(class_name)            \
    class_name(const class_name&) = delete;     \
    class_name(class_name&&) = delete;          \
    void operator=(const class_name&) = delete; \
    void operator=(class_name&&) = delete;


#endif // MACRO_H
