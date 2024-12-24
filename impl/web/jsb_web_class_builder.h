#ifndef GODOTJS_WEB_CLASS_BUILDER_H
#define GODOTJS_WEB_CLASS_BUILDER_H
#include "jsb_web_pch.h"
#include "jsb_web_class.h"
#include "jsb_web_handle_scope.h"
#include "jsb_web_primitive.h"
#include "jsb_web_template.h"
#include "jsb_web_function_interop.h"
#include "jsb_web_helper.h"

#define JSB_NEW_FUNCTION_TEMPLATE(isolate, name, callback, data) jsb::impl::Helper::NewFunctionTemplate(isolate, name, callback, data)

namespace jsb::impl
{
    namespace impl_private
    {
        template<typename T> struct Data {};

        template<> struct Data<int32_t> { static v8::Local<v8::Value> New(v8::Isolate* isolate, int32_t value) { return v8::Int32::New(isolate, value); } };
        template<> struct Data<int64_t> { static v8::Local<v8::Value> New(v8::Isolate* isolate, int64_t value) { return Helper::new_integer(isolate, value); } };
        template<> struct Data<uint32_t> { static v8::Local<v8::Value> New(v8::Isolate* isolate, uint32_t value) { return v8::Uint32::NewFromUnsigned(isolate, value); } };
        template<> struct Data<void*> { static v8::Local<v8::Value> New(v8::Isolate* isolate, void* value) { return v8::External::New(isolate, value); } };
        template<> struct Data<v8::Local<v8::Value>> { static v8::Local<v8::Value> New(v8::Isolate* isolate, v8::Local<v8::Value> value) { return value; } };
    }

    class ClassBuilder
    {
    private:
        v8::Isolate* isolate_ = nullptr;
        v8::Local<v8::FunctionTemplate> template_;
        v8::Local<v8::ObjectTemplate> prototype_template_;

        int internal_field_count_ = 0;
        bool closed_ = false;

    public:
        struct EnumDeclaration
        {
            EnumDeclaration(ClassBuilder* builder, bool is_instance_method, const v8::Local<v8::Name> name)
            : builder_(builder), enumeration_(v8::Object::New(builder_->isolate_))
            {
                jsb_check(!builder_->closed_);

                if (is_instance_method) builder_->prototype_template_->Set(builder_->GetContext(), name, enumeration_);
                else builder_->template_->Set(builder_->GetContext(), name, enumeration_);
            }

            EnumDeclaration& Value(const String& name, int64_t data)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);
                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::Value> value = impl_private::Data<int64_t>::New(builder_->isolate_, data);

                enumeration_->Set(builder_->GetContext(), key, value);

                // represents the value back to string for convenient uses, such as MyColor[MyColor.White] => 'White'
                jsbi_DefineProperty(builder_->isolate_->rt(), enumeration_->stack_pos_,
                    value->stack_pos_, // value as key
                    key->stack_pos_,   // key as value
                    StackBase::Undefined,
                    StackBase::Undefined,
                    PropertyFlags::ENUMERABLE | PropertyFlags::CONFIGURABLE | PropertyFlags::WRITABLE);
                return *this;
            }

        private:
            ClassBuilder* builder_;
            v8::Local<v8::Object> enumeration_;
        };

        struct MemberDeclaration
        {
            MemberDeclaration(ClassBuilder* builder, bool is_instance_method) : builder_(builder), is_instance_method(is_instance_method) {}

            EnumDeclaration Enum(const String& name)
            {
                return EnumDeclaration(builder_, is_instance_method, Helper::new_string(builder_->isolate_, name));
            }

            template<size_t N>
            void Method(const char (&name)[N], const v8::FunctionCallback callback)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::FunctionTemplate> value = JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, callback, {});

                if (is_instance_method) builder_->prototype_template_->Set(builder_->GetContext(), key, value);
                else builder_->template_->Set(builder_->GetContext(), key, value);
            }

            template<typename T>
            void Method(const String& name, const v8::FunctionCallback callback, T data)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::FunctionTemplate> value = JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, callback, impl_private::Data<T>::New(builder_->isolate_, data));

                if (is_instance_method) builder_->prototype_template_->Set(builder_->GetContext(), key, value);
                else builder_->template_->Set(builder_->GetContext(), key, value);
            }

            // getter/setter with common data payload
            template<typename T>
            void Property(const String& name, const v8::FunctionCallback getter_cb, const v8::FunctionCallback setter_cb, T data)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::Value> payload = impl_private::Data<T>::New(builder_->isolate_, data);
                const v8::Local<v8::FunctionTemplate> getter = getter_cb \
                    ? JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, getter_cb, payload)
                    : v8::Local<v8::FunctionTemplate>();
                const v8::Local<v8::FunctionTemplate> setter = setter_cb \
                    ? JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, setter_cb, payload)
                    : v8::Local<v8::FunctionTemplate>();;

                if (is_instance_method) builder_->prototype_template_->SetAccessorProperty(key, getter, setter);
                else builder_->template_->SetAccessorProperty(key, getter, setter);
            }

            template<typename GetterDataT, typename SetterDataT>
            void Property(const String& name, const v8::FunctionCallback getter_cb, GetterDataT getter_data, const v8::FunctionCallback setter_cb, SetterDataT setter_data)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::FunctionTemplate> getter = getter_cb \
                    ? JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, getter_cb, impl_private::Data<GetterDataT>::New(builder_->isolate_, getter_data))
                    : v8::Local<v8::FunctionTemplate>();
                const v8::Local<v8::FunctionTemplate> setter = setter_cb \
                    ? JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, setter_cb, impl_private::Data<SetterDataT>::New(builder_->isolate_, setter_data))
                    : v8::Local<v8::FunctionTemplate>();

                if (is_instance_method) builder_->prototype_template_->SetAccessorProperty(key, getter, setter);
                else builder_->template_->SetAccessorProperty(key, getter, setter);
            }

            template<typename GetterDataT>
            void Property(const String& name, const v8::FunctionCallback getter_cb, GetterDataT getter_data)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::FunctionTemplate> getter = getter_cb \
                    ? JSB_NEW_FUNCTION_TEMPLATE(builder_->isolate_, name, getter_cb, impl_private::Data<GetterDataT>::New(builder_->isolate_, getter_data))
                    : v8::Local<v8::FunctionTemplate>();

                if (is_instance_method) builder_->prototype_template_->SetAccessorProperty(key, getter);
                else builder_->template_->SetAccessorProperty(key, getter);
            }

            void LazyProperty(const String& name, const v8::AccessorNameGetterCallback getter)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);

                if (is_instance_method) builder_->prototype_template_->SetLazyDataProperty(builder_->GetContext(), key, getter);
                else builder_->template_->SetLazyDataProperty(builder_->GetContext(), key, getter);
            }

            template<typename T>
            void Value(const v8::Local<v8::Name> key, T val)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Value> value = impl_private::Data<T>::New(builder_->isolate_, val);
                if (is_instance_method) builder_->prototype_template_->Set(builder_->GetContext(), key, value);
                else builder_->template_->Set(builder_->GetContext(), key, value);
            }

            // generic set
            template<typename T>
            void Value(const String& name, T val)
            {
                jsb_check(!builder_->closed_);
                v8::HandleScope handle_scope(builder_->isolate_);

                const v8::Local<v8::Name> key = Helper::new_string(builder_->isolate_, name);
                const v8::Local<v8::Value> value = impl_private::Data<T>::New(builder_->isolate_, val);

                if (is_instance_method) builder_->prototype_template_->Set(builder_->GetContext(), key, value);
                else builder_->template_->Set(builder_->GetContext(), key, value);
            }

        private:
            ClassBuilder* builder_;
            bool is_instance_method;
        };

        ClassBuilder(const ClassBuilder&) = delete;
        ClassBuilder& operator=(const ClassBuilder&) = delete;

        ClassBuilder(ClassBuilder&&) noexcept = default;
        ClassBuilder& operator=(ClassBuilder&&) = default;

        MemberDeclaration Static() { return MemberDeclaration(this, false); }
        MemberDeclaration Instance() { return MemberDeclaration(this, true); }

        void Inherit(const Class& base)
        {
            jsb_check(!closed_);
            jsb_check(!base.IsEmpty());
            jsbi_SetPrototype(isolate_->rt(), template_->stack_pos_, base.prototype_.Get(isolate_)->stack_pos_);
        }

        Class Build()
        {
            jsb_checkf(!closed_, "class builder is already closed");
            closed_ = true;
            jsbi_SetConstructor(isolate_->rt(), template_->stack_pos_, prototype_template_->stack_pos_);
            return Class(isolate_, prototype_template_, template_);
        }

        template<uint8_t InternalFieldCount>
        static ClassBuilder New(v8::Isolate* isolate, const StringName& name, const v8::FunctionCallback constructor, const uint32_t class_payload)
        {
            //NOTE do not use HandleScope here, because prototype/constructor Local handles are temporarily saved
            //     in member fields of builder.

            ClassBuilder builder;
            const String str = name;
            const CharString str8 = str.utf8();

            jsb_checkf(str.length(), "empty string is not allowed for a class name");
            builder.internal_field_count_ = InternalFieldCount;
            builder.isolate_ = isolate;
            builder.prototype_template_ = v8::Local<v8::ObjectTemplate>(v8::Data(isolate, jsbi_NewObject(isolate->rt())));
            builder.template_ = v8::Local<v8::FunctionTemplate>(v8::Data(isolate, jsbi_NewClass(isolate->rt(),
                /* callback */ (jsb::impl::FunctionPointer) constructor,
                /* data */ jsbi_NewUint32(isolate->rt(), class_payload),
                (int) InternalFieldCount)));

            return builder;
        }

        ~ClassBuilder() = default;

    private:
        v8::Local<v8::Context> GetContext() const
        {
            return isolate_->GetCurrentContext();
        }

        ClassBuilder() {}
    };

}
#endif
