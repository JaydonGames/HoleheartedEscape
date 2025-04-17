#pragma once
#include <vector>

namespace Tiled {
    template<typename T>
    class Grid {
    public:
        Grid(){
            this->row_size = 0;
        }

        Grid(size_t row_size, size_t col_size){
            this->row_size = row_size;
            data.resize(row_size*col_size);
        }

        class Row {
        public:
            using iterator = T*;

            inline iterator begin(){
                return b;
            }

            inline iterator end(){
                return e;
            }

            inline T& operator [](size_t i){
                return *(this->b+i);
            }

            inline Row& operator *(){
                return *this;
            }

            inline void operator ++(){
                *this += 1;
            }

            inline void operator --(){
                *this -= 1;
            }

            inline void operator +=(size_t i){
                this->b += i*this->row_size;
                this->e += i*this->row_size;
            }

            inline void operator -=(size_t i){
                this->b -= i*this->row_size;
                this->e -= i*this->row_size;
            }

            inline bool operator ==(Row other){
                return this->b == other.b;
            }

            inline bool operator !=(Row other){
                return this->b != other.b;
            }

            size_t size(){
                return this->row_size;
            }

        private:
            iterator b, e;
            size_t row_size;
            friend Grid;
            Row(iterator b, iterator e, size_t row_size) : b(b), e(e), row_size(row_size) {}
        };

        using iterator = Row;


        size_t size(){
            return this->data.size() / this->row_size;
        }

        inline Row operator [](size_t i){
            typename Row::iterator b = data.data() + i*row_size;
            return {b, b+row_size, this->row_size};
        }

        inline Row begin(){
            return (*this)[0];
        }

        inline Row end(){
            return (*this)[this->size()];
        }


    private:
        std::vector<T> data;
        size_t row_size;
    };
}
