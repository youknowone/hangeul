//
//  type.h
//  hangeul
//
//  Created by Jeong YunWon on 2014. 6. 8..
//  Copyright (c) 2014년 youknowone.org. All rights reserved.
//

#ifndef __HANGEUL_TYPE__
#define __HANGEUL_TYPE__

#include <stdint.h>
#include <vector>
#include <map>
#include <list>
#include <cassert>

namespace hangeul {
    typedef uint32_t InputSource;
    typedef uint32_t Unicode;
    typedef std::vector<Unicode> UnicodeVector;
    typedef uint32_t KeyStroke;

    class State: public std::map<int32_t, int32_t> {
    public:
        class ArrayProxy {
            State *owner;
            int32_t head;
            size_t max_size;
        public:
            void set_size(size_t size) {
                (*this->owner)[this->head - 1] = (int32_t)size;
            }
        public:
            class Iterator: public std::iterator<std::bidirectional_iterator_tag, uint32_t> {
                ArrayProxy *_array;
                int32_t _idx;
            public:
                Iterator(ArrayProxy *array, int32_t idx): _array(array), _idx(idx) {}
                Iterator(const Iterator& it) : _array(it._array), _idx(it._idx) {}
                Iterator& operator++() { ++_idx; return *this; }
                Iterator operator++(int) { Iterator tmp(*this); operator++(); return tmp; }
                bool operator==(const Iterator& rhs) { return _array == rhs._array && _idx == rhs._idx; }
                bool operator!=(const Iterator& rhs) { return _array != rhs._array || _idx != rhs._idx; }
                int& operator*() { return (*_array)[_idx]; }
            };
            ArrayProxy(State *owner, int32_t head, ssize_t maximum_size = -1) {
                this->owner = owner;
                this->head = head;
                this->max_size = maximum_size;
            }
            size_t size() {
                auto size = (*this->owner)[this->head - 1];
                return size;
            }
            int32_t& operator[](ssize_t subscript) {
                if (subscript >= 0) {
                    return (*this->owner)[(int32_t)(this->head + subscript)];
                } else {
                    return (*this->owner)[(int32_t)(this->head + this->size() + subscript)];
                }
            }
            Iterator begin() {
                return Iterator(this, 0);
            }
            Iterator end() {
                return Iterator(this, this->size());
            }
            int32_t& back() {
                auto size = this->size();
                auto& value = (*this)[size - 1];
                return value;
            }
            void push_back(int32_t value) {
                auto size = this->size();
                (*this)[size] = value;
                this->set_size(size + 1);
                assert(size + 1 <= max_size);
            }
            void pop_back() {
                auto size = this->size();
                this->set_size(size - 1);
            }
            void erase(uint32_t from_idx, size_t length = 1) { // very expensive
                auto size = this->size();
                for (uint32_t i = from_idx + length; i < size; i++) {
                    (*this->owner)[i - length] = (*this->owner)[i];
                }
                this->set_size(size - length);
            }
            void import(ArrayProxy& other) {
                this->set_size(other.size());
                for (int i = 0; i < other.size(); i++) {
                    (*this)[i] = other[i];
                }
            }
        };
    public:
        ArrayProxy array(int32_t idx) {
            return ArrayProxy(this, idx);
        }
        InputSource latestInputSource() {
            return (*this)[-1];
        }
        KeyStroke latestKeyStroke() {
            return (*this)[0];
        }
        #if DEBUG
        void _debug() {
            printf("-- debug info --\n");
            for (auto& it: *this) {
                printf("%lx(%ld): %lx(%ld)\n", it.first, it.first, it.second, it.second);
            }
            printf("-- end of info --\n");
        }
        #endif
    };

    enum KeyPosition { // platform independent position
        KeyPositionGrave = 0x00,
        KeyPosition1 = 0x01,
        KeyPosition2,
        KeyPosition3,
        KeyPosition4,
        KeyPosition5,
        KeyPosition6,
        KeyPosition7,
        KeyPosition8,
        KeyPosition9,
        KeyPosition0,
        KeyPositionMinus,
        KeyPositionEqual,
        KeyPositionBackslash, // 0x0d
        KeyPositionBackspace, // 0x0e
        KeyPositionTab = 0x10,
        KeyPositionQ = 0x11,
        KeyPositionW,
        KeyPositionE,
        KeyPositionR,
        KeyPositionT,
        KeyPositionY,
        KeyPositionU,
        KeyPositionI,
        KeyPositionO,
        KeyPositionP,
        KeyPositionBracketLeft,
        KeyPositionBracketRight, // 0x1c
        KeyPositionA = 0x21,
        KeyPositionS,
        KeyPositionD,
        KeyPositionF,
        KeyPositionG,
        KeyPositionH,
        KeyPositionJ,
        KeyPositionK,
        KeyPositionL,
        KeyPositionColon,
        KeyPositionQuote,
        KeyPositionEnter, // 0x2b
        KeyPositionLeftShift = 0x30,
        KeyPositionZ = 0x31,
        KeyPositionX,
        KeyPositionC,
        KeyPositionV,
        KeyPositionB,
        KeyPositionN,
        KeyPositionM,
        KeyPositionComma,
        KeyPositionPeriod,
        KeyPositionSlash, // 0x3a
        KeyPositionEsc = 0x3e,
        KeyPositionRightShift = 0x3f,
        KeyPositionSpace = 0x40,
        KeyPositionDelete = 0x41,
        KeyPositionLeft = 0x42,
        KeyPositionRight,
        KeyPositionUp,
        KeyPositionDown,
        KeyPositionHome = 0x46,
        KeyPositionEnd,
        KeyPositionPageUp,
        KeyPositionPageDown,
        KeyPositionLeftControl = 0x4a,
        KeyPositionLeftOS,
        KeyPositionLeftAlt,
        KeyPositionRightAlt,
        KeyPositionRightOS,
        KeyPositionRightControl, // 0x4f
        KeyPositionPad0 = 0x50,
        KeyPositionPad1,
        KeyPositionPad2,
        KeyPositionPad3,
        KeyPositionPad4,
        KeyPositionPad5,
        KeyPositionPad6,
        KeyPositionPad7,
        KeyPositionPad8,
        KeyPositionPad9,
        KeyPositionPadDecimal = 0x5f,
        KeyPositionPadPeriod = 0x60,
        KeyPositionPadPlus,
        KeyPositionPadMinus,
        KeyPositionPadMultiply,
        KeyPositionPadDivide,
        KeyPositionPadEqual,
        KeyPositionPadEnter,
        KeyPositionPadClear,
        KeyPositionISOSection = 0xd0,
        KeyPositionYen = 0xd2,
        KeyPositionUnderscore,
        KeyPositionPadComma,
        KeyPositionEisu,
        KeyPositionKana,
        KeyPositionFunction = 0xe0,
        KeyPositionF1 = 0xe1,
        KeyPositionF2,
        KeyPositionF3,
        KeyPositionF4,
        KeyPositionF5,
        KeyPositionF6,
        KeyPositionF7,
        KeyPositionF8,
        KeyPositionF9,
        KeyPositionF10,
        KeyPositionF11,
        KeyPositionF12,
        KeyPositionF13,
        KeyPositionF14,
        KeyPositionF15,
        KeyPositionF16,
        KeyPositionF17,
        KeyPositionF18,
        KeyPositionF19,
        KeyPositionF20,
    };

    typedef uint32_t KeyStroke;
    #define KeyStrokeBackspace 0x0e
    //KeyStroke KeyStrokeFromPositionAndModifiers(KeyPosition position, bool modifiers[5]) {
    //    return (modifiers[0] << 16) + (modifiers[1] << 17) + (modifiers[2] << 18) + (modifiers[3] << 19) + (modifiers[4] << 20) + position; // altshift, shift, control, alt, OS
    //}

}

#endif