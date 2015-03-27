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

    template <class Itr>
    class CharItr
    {
        static_assert(std::is_same<typename std::iterator_traits<Itr>::value_type, char>::value, "char iterator expected");

        Itr itr;
        Itr end;

    public:
        explicit CharItr(const Itr &begin_, const Itr &end_) :
            itr(begin_),
            end(end_)
        {
        }

        char operator *() const
        {
            if( itr != end )
                return *itr;

            return 0;
        }

        void operator ++()
        {
            if( itr != end )
                ++itr;
        }
    };

    template <class Itr>
    class Reader
    {
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

        static void SkipWhitespace(Itr &itr)
        {
            while( IsWhitespace(*itr) )
                ++itr;
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
        static bool TryReadExpectedString(Itr &itr, const char(&pStr)[N])
        {
            static_assert(N > 0, "internal error: string not null-terminated");

            for(std::size_t i = 0; i < (N - 1); ++i, ++itr)
                if( *itr != pStr[i] )
                    return false;

            return true;
        }

        static double ReadNumber(Itr &itr)
        {
            int sign = 1;
            if( *itr == '-' )
            {
                sign = -1;
                ++itr;
            }

            double number = 0;
            for( ; *itr; ++itr )
            {
                switch( *itr )
                {
                    case TinyJson_Digits_0_9:
                        number = number * 10 + (*itr - '0');
                        continue;
                }

                break;
            }

            if( *itr == '.' )
            {
                ++itr;

                int factor = 1;
                for( ; *itr; ++itr, factor *= 10 )
                {
                    switch( *itr )
                    {
                        case TinyJson_Digits_0_9:
                            number = number * 10 + (*itr - '0');
                            continue;
                    }

                    break;
                }

                number /= factor;
            }

            if( *itr == 'e' || *itr == 'E' )
            {
                ++itr;

                bool eNegative = false;
                if( *itr == '+' )
                {
                    ++itr;
                }
                else if( *itr == '-' )
                {
                    eNegative = true;
                    ++itr;
                }

                int e = 0;
                for( ; *itr; ++itr )
                {
                    switch( *itr )
                    {
                        case TinyJson_Digits_0_9:
                            e = e * 10 + (*itr - '0');
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

        static void ReadExpectedChar(Itr &itr, const char ch)
        {
            if( *itr != ch )
                throw std::runtime_error("'" + std::string(1, ch) + "' expected");

            ++itr;
        }

        static String ReadString(Itr &itr)
        {
            assert(*itr == '"');
            ++itr;

            String str;

            for( ; *itr; ++itr )
            {
                if( *itr == '\\' )
                {
                    ++itr;
                    switch( *itr )
                    {
                        case '"':
                        case '\\':
                        case '/':
                            str.push_back(*itr);
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
                            throw std::runtime_error("unrecognized character escape sequence: \\" + std::string(1, *itr));
                            break;
                    }
                }
                else if( *itr == '"' )
                {
                    break;
                }
                else
                {
                    str.push_back(*itr);
                }
            }

            ReadExpectedChar(itr, '"');
            return str;
        }

        static Array ReadArray(Itr &itr)
        {
            assert(*itr == '[');
            ++itr;

            Array arr;

            for( ;; )
            {
                arr.push_back(ReadValue(itr));

                SkipWhitespace(itr);
                if( *itr == ',' )
                {
                    ++itr;
                    continue;
                }

                ReadExpectedChar(itr, ']');
                break;
            }

            return arr;
        }

        static std::string ReadKey(Itr &itr)
        {
            SkipWhitespace(itr);

            if( *itr != '"' )
                throw std::runtime_error("string expected");

            return ReadString(itr);
        }

        static Object ReadObject(Itr &itr)
        {
            assert(*itr == '{');
            ++itr;

            Object obj;

            for( ;; )
            {
                auto key = ReadKey(itr);

                SkipWhitespace(itr);
                ReadExpectedChar(itr, ':');

                auto value = ReadValue(itr);

                if( !obj.emplace(std::move(key), std::move(value)).second )
                    throw std::runtime_error("duplicate key: " + key);

                SkipWhitespace(itr);
                if( *itr == ',' )
                {
                    ++itr;
                    continue;
                }

                ReadExpectedChar(itr, '}');
                break;
            }

            return obj;
        }

    public:
        static ValuePtr ReadValue(Itr &itr)
        {
            SkipWhitespace(itr);

            switch( *itr )
            {
                case '-':
                case TinyJson_Digits_0_9:
                    return CreateValue<NumberValue>(ReadNumber(itr));

                case '"':
                    return CreateValue<StringValue>(ReadString(itr));

                case '[':
                    return CreateValue<ArrayValue>(ReadArray(itr));

                case '{':
                    return CreateValue<ObjectValue>(ReadObject(itr));

                case 't':
                {
                    if( TryReadExpectedString(itr, "true") )
                        return CreateValue<BooleanValue>(true);
                }
                break;

                case 'f':
                {
                    if( TryReadExpectedString(itr, "false") )
                        return CreateValue<BooleanValue>(false);
                }
                break;

                case 'n':
                {
                    if( TryReadExpectedString(itr, "null") )
                        return std::unique_ptr<NullValue>(new NullValue());
                }
                break;
            }

            throw std::runtime_error("Invalid format");
        }
    };

    template <class Itr>
    CharItr<Itr> MakeStream(const Itr &begin, const Itr &end)
    {
        return CharItr<Itr>(begin, end);
    }

    inline CharItr<const char *> MakeStream(const char *const pStr)
    {
        return MakeStream(pStr, pStr + std::strlen(pStr));
    }

    template <class Itr>
    ValuePtr Read(CharItr<Itr> &stream)
    {
        return Reader<CharItr<Itr>>::ReadValue(stream);
    }

    template <class Itr>
    ValuePtr Read(const Itr &begin, const Itr &end)
    {
        auto stream = MakeStream(begin, end);
        return Read(stream);
    }

    inline ValuePtr Read(const char *const pStr)
    {
        auto stream = MakeStream(pStr);
        return Read(stream);
    }

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
