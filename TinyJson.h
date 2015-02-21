#pragma once

#include <memory>
#include <vector>
#include <map>
#include <cassert>
#include <deque>
#include <list>
#include <unordered_map>

namespace TinyJson
{
    enum class ValueType
    {
        Null,
        Number,
        String,
        Array,
        Object,
        Boolean
    };

    struct ValueBase;

    using ValuePtr = std::unique_ptr<ValueBase>;

    using Number = double;
    using String = std::string;
    using Array = std::vector<ValuePtr>;
    using Object = std::map<std::string, ValuePtr>;
    using Boolean = bool;

    struct ValueBase
    {
        explicit ValueBase() =default;
        virtual ~ValueBase() =0;

        explicit ValueBase(const ValueBase &) =delete;
        void operator =(const ValueBase &) =delete;

        virtual ValueType Type() const =0;

        bool IsNull() const { return Type() == ValueType::Null; }
        bool IsNumber() const { return Type() == ValueType::Number; }
        bool IsString() const { return Type() == ValueType::String; }
        bool IsArray() const { return Type() == ValueType::Array; }
        bool IsObject() const { return Type() == ValueType::Object; }
        bool IsBoolean() const { return Type() == ValueType::Boolean; }

        virtual const Number &AsNumber() const { throw std::runtime_error("value is not a number"); }
        virtual const String &AsString() const { throw std::runtime_error("value is not a string"); }
        virtual const Array &AsArray() const { throw std::runtime_error("value is not an array"); }
        virtual const Object &AsObject() const { throw std::runtime_error("value is not an object"); }
        virtual const Boolean &AsBoolean() const { throw std::runtime_error("value is not a boolean"); }
    };

    inline ValueBase::~ValueBase() =default;

    struct NullValue : public ValueBase
    {
        ValueType Type() const override { return ValueType::Null; }
    };

    struct NumberValue : public ValueBase
    {
        Number value;

        ValueType Type() const override { return ValueType::Number; }
        const Number &AsNumber() const override { return value; }
    };

    struct StringValue : public ValueBase
    {
        String value;

        ValueType Type() const override { return ValueType::String; }
        const String &AsString() const override { return value; }
    };

    struct ArrayValue : public ValueBase
    {
        Array value;

        ValueType Type() const override { return ValueType::Array; }
        const Array &AsArray() const override { return value; }
    };

    struct ObjectValue : public ValueBase
    {
        Object value;

        ValueType Type() const override { return ValueType::Object; }
        const Object &AsObject() const override { return value; }
    };

    struct BooleanValue : public ValueBase
    {
        Boolean value;

        ValueType Type() const override { return ValueType::Boolean; }
        const Boolean &AsBoolean() const override { return value; }
    };

#define TinyJson_Digits_0_9 \
         '0': \
    case '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9'

    class Reader
    {
        const char *m_pData;

        static bool IsWhitespace(const char ch)
        {
            switch( ch )
            {
                case ' ':
                case '\f':
                case '\n':
                case '\r':
                case '\t':
                case '\v':
                    return true;
            }

            return false;
        }

        static void SkipWhitespace(const char *&pData)
        {
            while( IsWhitespace(*pData) )
                ++pData;
        }

        template <class T, class U>
        static std::unique_ptr<T> CreateValue(U &&value)
        {
            static_assert(std::is_base_of<ValueBase, T>::value, "internal error: invalid type passed to CreateValue");

            std::unique_ptr<T> ptr(new T());
            ptr->value = std::move(value);

            return ptr;
        }

        template <std::size_t N>
        static bool TryReadExpectedString(const char *&pData, const char(&pStr)[N])
        {
            static_assert(N > 0, "internal error: string not null-terminated");

            if( strncmp(pData, pStr, (N - 1)) != 0 )
                return false;

            pData += (N - 1);
            return true;
        }

        static ValuePtr ReadValue(const char *&pData)
        {
            SkipWhitespace(pData);

            switch( *pData )
            {
                case '-':
                case TinyJson_Digits_0_9:
                    return CreateValue<NumberValue>(ReadNumber(pData));

                case '"':
                    return CreateValue<StringValue>(ReadString(pData));

                case '[':
                    return CreateValue<ArrayValue>(ReadArray(pData));

                case '{':
                    return CreateValue<ObjectValue>(ReadObject(pData));

                case 't':
                {
                    if( TryReadExpectedString(pData, "true") )
                        return CreateValue<BooleanValue>(true);
                }
                break;

                case 'f':
                {
                    if( TryReadExpectedString(pData, "false") )
                        return CreateValue<BooleanValue>(false);
                }
                break;

                case 'n':
                {
                    if( TryReadExpectedString(pData, "null") )
                        return std::unique_ptr<NullValue>(new NullValue());
                }
                break;
            }

            throw std::runtime_error("Invalid format");
        }

        static double ReadNumber(const char *&pData)
        {
            int sign = 1;
            if( *pData == '-' )
            {
                sign = -1;
                ++pData;
            }

            double number = 0;
            for( ; *pData; ++pData )
            {
                switch( *pData )
                {
                    case TinyJson_Digits_0_9:
                        number = number * 10 + (*pData - '0');
                        continue;
                }

                break;
            }

            if( *pData == '.' )
            {
                ++pData;

                int factor = 1;
                for( ; *pData; ++pData, factor *= 10 )
                {
                    switch( *pData )
                    {
                        case TinyJson_Digits_0_9:
                            number = number * 10 + (*pData - '0');
                            continue;
                    }

                    break;
                }

                number /= factor;
            }

            if( *pData == 'e' || *pData == 'E' )
            {
                ++pData;

                bool eNegative = false;
                if( *pData == '+' )
                {
                    ++pData;
                }
                else if( *pData == '-' )
                {
                    eNegative = true;
                    ++pData;
                }

                int e = 0;
                for( ; *pData; ++pData )
                {
                    switch( *pData )
                    {
                        case TinyJson_Digits_0_9:
                            e = e * 10 + (*pData - '0');
                            continue;
                    }

                    break;
                }

                int power = 1;
                for( int i = 0; i < e; ++i )
                    power *= 10;

                if( eNegative )
                    number /= power;
                else
                    number *= power;
            }

            number *= sign;

            return number;
        }

        static void ReadExpectedChar(const char *&pData, const char ch)
        {
            if( *pData != ch )
                throw std::runtime_error("'" + std::string(1, ch) + "' expected");

            ++pData;
        }

        static String ReadString(const char *&pData)
        {
            assert(*pData == '"');
            ++pData;

            String str;

            for( ; *pData; ++pData )
            {
                if( *pData == '\\' )
                {
                    ++pData;
                    switch( *pData )
                    {
                        case '"':
                        case '\\':
                        case '/':
                            str.push_back(*pData);
                            break;

                        case 'b':str.push_back('\b'); break;
                        case 'f':str.push_back('\f'); break;
                        case 'n':str.push_back('\n'); break;
                        case 'r':str.push_back('\r'); break;
                        case 't':str.push_back('\t'); break;

                        case 'u':
                            throw std::runtime_error("\\u control character not implemented");
                            break;

                        default:
                            throw std::runtime_error("unrecognized character escape sequence: \\" + std::string(1, *pData));
                            break;
                    }
                }
                else if( *pData == '"' )
                {
                    break;
                }
                else
                {
                    str.push_back(*pData);
                }
            }

            ReadExpectedChar(pData, '"');
            return str;
        }

        static Array ReadArray(const char *&pData)
        {
            assert(*pData == '[');
            ++pData;

            Array arr;

            for( ;; )
            {
                arr.push_back(ReadValue(pData));

                SkipWhitespace(pData);
                if( *pData == ',' )
                {
                    ++pData;
                    continue;
                }

                ReadExpectedChar(pData, ']');
                break;
            }

            return arr;
        }

        static std::string ReadKey(const char *&pData)
        {
            SkipWhitespace(pData);

            if( *pData != '"' )
                throw std::runtime_error("string expected");

            return ReadString(pData);
        }

        static Object ReadObject(const char *&pData)
        {
            assert(*pData == '{');
            ++pData;

            Object obj;

            for( ;; )
            {
                auto key = ReadKey(pData);

                SkipWhitespace(pData);
                ReadExpectedChar(pData, ':');

                auto value = ReadValue(pData);

                if( !obj.emplace(std::move(key), std::move(value)).second )
                    throw std::runtime_error("duplicate key: " + key);

                SkipWhitespace(pData);
                if( *pData == ',' )
                {
                    ++pData;
                    continue;
                }

                ReadExpectedChar(pData, '}');
                break;
            }

            return obj;
        }

    public:
        explicit Reader(const char *const pData) :
            m_pData(pData)
        {
            if( !pData )
                throw std::invalid_argument("data is null");
        }

        ValuePtr Read()
        {
            return ReadValue(m_pData);
        }
    };

    template <class T>
    struct ConvertTo;

    template <class T>
    struct ConvertToNumber
    {
        static T From(const ValuePtr &value)
        {
            return static_cast<T>(value->AsNumber());
        }
    };

#define TinyJson_DefineConvertToNumber(X) \
    template <> struct ConvertTo<X> : public ConvertToNumber<X> { }

    TinyJson_DefineConvertToNumber(int);
    TinyJson_DefineConvertToNumber(unsigned int);
    TinyJson_DefineConvertToNumber(short);
    TinyJson_DefineConvertToNumber(unsigned short);
    TinyJson_DefineConvertToNumber(long);
    TinyJson_DefineConvertToNumber(unsigned long);
    TinyJson_DefineConvertToNumber(float);
    TinyJson_DefineConvertToNumber(double);

    template <>
    struct ConvertTo<std::string>
    {
        static std::string From(const ValuePtr &value)
        {
            return value->AsString();
        }
    };

    template <class Container>
    struct ConvertToSequenceContainer
    {
        static Container From(const ValuePtr &value)
        {
            Container result;

            const auto &arr = value->AsArray();
            result.reserve(arr.size());

            for( const auto &item: arr )
                result.push_back(Convert<Container::value_type>(item));

            return result;
        }
    };

#define TinyJson_DefineConvertToSequenceContainer(X) \
    template <class T> struct ConvertTo<X<T>> : public ConvertToSequenceContainer<X<T>> { }

    TinyJson_DefineConvertToSequenceContainer(std::vector);
    TinyJson_DefineConvertToSequenceContainer(std::deque);
    TinyJson_DefineConvertToSequenceContainer(std::list);

    template <class Container>
    struct ConvertToAssociativeContainer
    {
        static Container From(const ValuePtr &value)
        {
            Container result;

            const auto &m = value->AsObject();
            for( const auto &entry: m )
                result.emplace(entry.first, Convert<Container::mapped_type>(entry.second));

            return result;
        }
    };

#define TinyJson_DefineConvertToAssociativeContainer(X) \
    template <class T> struct ConvertTo<X<std::string, T>> : public ConvertToAssociativeContainer<X<std::string, T>> { }

    TinyJson_DefineConvertToAssociativeContainer(std::map);
    TinyJson_DefineConvertToAssociativeContainer(std::unordered_map);

    template <>
    struct ConvertTo<bool>
    {
        static bool From(const ValuePtr &value)
        {
            return value->AsBoolean();
        }
    };

    template <class T, class U>
    struct ConvertTo<std::pair<T, U>>
    {
        static std::pair<T, U> From(const ValuePtr &value)
        {
            const auto &arr = value->AsArray();
            if( arr.size() != 2 )
                throw std::runtime_error("pair must contain exactly two items");

            return std::make_pair(Convert<T>(arr[0]), Convert<U>(arr[1]));
        }
    };

    template <class T>
    T Convert(const ValuePtr &value)
    {
        return ConvertTo<T>::From(value);
    }
}
