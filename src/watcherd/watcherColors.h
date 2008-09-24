#ifndef WATCHER_COLORS_H
#define WATCHER_COLORS_H

#include <iostream>
#include <boost/shared_ptr.hpp>
#include "logger.h"

namespace boost {
    namespace archive {
        class polymorphic_iarchive;
        class polymorphic_oarchive;
    }
}

namespace watcher
{
    // 
    // Watcher color is RGB+alpha
    //
    class Color
    {
        public:

            //
            // A few default colors. Plenty more at: 
            // http://htmlhelp.com/cgi-bin/color.cgi
            // Feel free to add to this list.
            //
            static const Color black;
            static const Color white;

            static const Color violet;
            static const Color indigo;
            static const Color blue;
            static const Color green;
            static const Color yellow;
            static const Color orange;
            static const Color red;

            static const Color darkblue;
            static const Color magenta;
            static const Color gold;
            static const Color turquoise;
            static const Color brown;
            static const Color deeppink;

            static std::string toString(const Color &c); 

            //
            // Now the real Color class begins...
            //
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;

            Color();
            Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a);
            Color(const uint32_t &color);       // color == rgba

            // color = "black", "white", etc or hex value, i.e. "0xff34ds00"
            // This constructor will throw boost::bad_lexical_cast if the string is 
            // not properly formatted (able to be converted into a uint32_t). 
            Color(const std::string &color);    

            Color(const Color &other);
            ~Color(); 

            bool operator==(const Color &c) const;
            Color &operator=(const Color &other);

            std::ostream &toStream(std::ostream &out) const;
            std::ostream &operator<<(std::ostream &out) const { return toStream(out); }

            void serialize(boost::archive::polymorphic_iarchive & ar, const unsigned int file_version);
            void serialize(boost::archive::polymorphic_oarchive & ar, const unsigned int file_version);

        protected:
        private:
            DECLARE_LOGGER();
    };

    // global operators and typedefs.
    typedef boost::shared_ptr<Color> ColorPtr;
    std::ostream &operator<<(std::ostream &out, const Color &c);
}
#endif // WATCHER_COLORS_H
