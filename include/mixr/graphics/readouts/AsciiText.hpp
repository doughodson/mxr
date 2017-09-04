
#ifndef __mixr_graphics_AsciiText_H__
#define __mixr_graphics_AsciiText_H__

#include "mixr/graphics/readouts/AbstractField.hpp"

namespace mixr {
namespace base { class Object; class List; class String; }
namespace graphics {

//------------------------------------------------------------------------------
// Class: AsciiText
//
// Factory name: text
// Slots:
//    text   <String>     ! Text String (default: " ")
//    text   <List>       ! Text List
//
// Public Member Functions:
//      bool isValidInputPosition(int tc)
//      Returns true if tc is a valid input position.
//
//      bool setTextString(String* stsobj)
//      Returns true if the text string was set to stsobj.
//
//      bool setTextList(List* stlobj)
//      Returns true if the text list was set to stlobj.
//
//------------------------------------------------------------------------------
class AsciiText : public AbstractField
{
    DECLARE_SUBCLASS(AsciiText, AbstractField)

public:
   AsciiText();

   virtual char filterInputEvent(const int event, const int tc) override;
   virtual bool isValidInputPosition(const int) override;
   virtual bool event(const int key, base::Object* const obj = nullptr) override;

   virtual bool setTextString(const base::String* const);
   virtual bool setTextList(const base::List* const);
};

}
}

#endif
