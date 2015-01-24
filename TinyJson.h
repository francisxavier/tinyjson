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
    struct Value;

    enum class ValueType
    {
        Null,
        Number,
        String,
        Array,
        Object,
        Boolean
    };

    typedef double NumberType;
    typedef std::string StringType;
    typedef std::vector<std::unique_ptr<Value>> ArrayType;
    typedef std::map<std::string, std::unique_ptr<Value>> ObjectType;
    typedef bool BooleanType;

    struct Value
    {
        explicit Value() =default;
        virtual ~Value() =0;

        explicit Value(const Value &) =delete;
        void operator =(const Value &) =delete;

        virtual ValueType Type() const =0;

        bool IsNull() const { return Type() == ValueType::Null; }
        bool IsNumber() const { return Type() == ValueType::Number; }
        bool IsString() const { return Type() == ValueType::String; }
        bool IsArray() const { return Type() == ValueType::Array; }
        bool IsObject() const { return Type() == ValueType::Object; }
        bool IsBoolean() const { return Type() == ValueType::Boolean; }

        virtual const NumberType &AsNumber() const { throw std::runtime_error("value is not a number"); }
        virtual const StringType &AsString() const { throw std::runtime_error("value is not a string"); }
        virtual const ArrayType &AsArray() const { throw std::runtime_error("value is not an array"); }
        virtual const ObjectType &AsObject() const { throw std::runtime_error("value is not an object"); }
        virtual const BooleanType &AsBoolean() const { throw std::runtime_error("value is not a boolean"); }
    };

    inline Value::~Value() { }

    struct Null : public Value
    {
        ValueType Type() const override { return ValueType::Null; }
    };

    struct Number : public Value
    {
        NumberType value;

        ValueType Type() const override { return ValueType::Number; }
        const NumberType &AsNumber() const override { return value; }
    };

    struct String : public Value
    {
        StringType value;

        ValueType Type() const override { return ValueType::String; }
        const StringType &AsString() const override { return value; }
    };

    struct Array : public Value
    {
        ArrayType value;

        ValueType Type() const override { return ValueType::Array; }
        const ArrayType &AsArray() const override { return value; }
    };

    struct Object : public Value
    {
        ObjectType value;

        ValueType Type() const override { return ValueType::Object; }
        const ObjectType &AsObject() const override { return value; }
    };

    struct Boolean : public Value
    {
        BooleanType value;

        ValueType Type() const override { return ValueType::Boolean; }
        const BooleanType &AsBoolean() const override { return value; }
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
            static_assert(std::is_base_of<Value, T>::value, "internal error: invalid type passed to CreateValue");

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

        static std::unique_ptr<Value> ReadValue(const char *&pData)
        {
            SkipWhitespace(pData);

            switch( *pData )
            {
                case '-':
                case TinyJson_Digits_0_9:
                    return CreateValue<Number>(ReadNumber(pData));

                case '"':
                    return CreateValue<String>(ReadString(pData));

                case '[':
                    return CreateValue<Array>(ReadArray(pData));

                case '{':
                    return CreateValue<Object>(ReadObject(pData));

                case 't':
                {
                    if( TryReadExpectedString(pData, "true") )
                        return CreateValue<Boolean>(true);
                }
                break;

                case 'f':
                {
                    if( TryReadExpectedString(pData, "false") )
                        return CreateValue<Boolean>(false);
                }
                break;

                case 'n':
                {
                    if( TryReadExpectedString(pData, "null") )
                        return std::unique_ptr<Null>(new Null());
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

        static std::string ReadString(const char *&pData)
        {
            assert(*pData == '"');
            ++pData;

            std::string str;

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

        static std::vector<std::unique_ptr<Value>> ReadArray(const char *&pData)
        {
            assert(*pData == '[');
            ++pData;

            std::vector<std::unique_ptr<Value>> arr;

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

        static std::map<std::string, std::unique_ptr<Value>> ReadObject(const char *&pData)
        {
            assert(*pData == '{');
            ++pData;

            std::map<std::string, std::unique_ptr<Value>> obj;

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

        std::unique_ptr<Value> Read()
        {
            return ReadValue(m_pData);
        }
    };

    template <class T>
    struct ConvertTo;

    template <class T>
    struct ConvertToNumber
    {
        static T From(const std::unique_ptr<Value> &value)
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
        static std::string From(const std::unique_ptr<Value> &value)
        {
            return value->AsString();
        }
    };

    template <class Container>
    struct ConvertToSequenceContainer
    {
        static Container From(const std::unique_ptr<Value> &value)
        {
            Container result;

            const auto &v = value->AsArray();
            result.reserve(v.size());

            for( const auto &item: v )
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
        static Container From(const std::unique_ptr<Value> &value)
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
        static bool From(const std::unique_ptr<Value> &value)
        {
            return value->AsBoolean();
        }
    };

    template <class T>
    T Convert(const std::unique_ptr<Value> &value)
    {
        return ConvertTo<T>::From(value);
    }
}
