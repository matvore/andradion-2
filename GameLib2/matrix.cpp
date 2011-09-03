#ifndef _77ADF340_46E9_11d4_B6FE_0050040B0541_INCLUDED_
#define _77ADF340_46E9_11d4_B6FE_0050040B0541_INCLUDED_

#include <vector>
#include <cassert>

using std::vector;

namespace NGameLib2 {
	template <class itemType> class matrix {
	public:
		// constructors/destructor
		matrix(); // empty matrix with 0x0 dim
		matrix(unsigned int rows,unsigned int columns_,const itemType& fill = itemType()); // all entries == fillValue
		matrix(const matrix& x);

		~matrix(); // destructor

		// assignment
		matrix& operator =(const matrix& rhs);

		// accessors
		int rows(void) const;
		int cols(void) const;

		// indexing
		const itemType * operator [] (unsigned int k) const;  // range-checked indexing
		itemType * operator [] (unsigned int k);              // range-checked indexing

		// modifiers
		void resize(unsigned int rows,unsigned int columns_); // is destructive

	private:
		int columns;                             
		vector<itemType *> data; 
	};

	template <class itemType>
		matrix<itemType>::matrix(const matrix& x) : columns(0), data(0)
	{
		*this = x;
	}

	template <class itemType>
	matrix<itemType>::matrix() : columns(0), data(0)
	{
	}

	template <class itemType>
	matrix<itemType>::matrix(unsigned int rows, unsigned int columns_, const itemType& fill) : columns(columns_), data(rows)
	{
		for(int i = 0; i < rows; i++)
		{
			this->data[i] = new itemType[this->columns];
			for(int j = 0; j < this->columns; j++)
			{
				this->data[i][j] = fill;
			}
		}
	}

	template <class itemType>
	matrix<itemType>::~matrix () 
	{
		this->resize(0,0);
	}

	template <class itemType>
	matrix<itemType>&
	matrix<itemType>::operator =(const matrix<itemType>& rhs)
	{
		if (this != &rhs) // don't assign to self!
		{                  
			this->resize(rhs.rows(),rhs.cols());
        
			for(int i = 0; i < this->data.size(); i++)
			{
				for(int j = 0; j < this->columns; j++)
				{
					this->data[i][j] = rhs.data[i][j];
				}
			}
		}
		return *this;       
	}

	template <class itemType>
	int matrix<itemType>::rows() const
	{
		return this->data.size();
	}

	template <class itemType>
	int matrix<itemType>::cols() const
	{
		return this->columns;
	}


	template <class itemType>
	void matrix<itemType>::resize(unsigned int rows, unsigned int columns_)
	{
      unsigned int i;

		// delete what we already have
		for(i = 0; i < this->data.size(); i++)
		{
			delete [] this->data[i];
		}

		// check if we are resizing to zero
		if(0 == rows || 0 == columns_)
		{
			this->columns = 0;
			this->data.resize(0);
			return;
		}

		this->columns = columns_;

		this->data.resize(rows);

		for(i=0; i < rows; i++)
		{
			this->data[i] = new itemType[columns_];
		}
	}

	template <class itemType>
	const itemType *matrix<itemType>::operator [] (unsigned int k) const
	{
		assert(0 != this->columns); // make sure we don't have a size of zero
		return this->data[k];
	}

	template <class itemType>
	itemType *matrix<itemType>::operator [](unsigned int k) 
	{
		assert(0 != this->columns); // make sure we don't have a size of zero
		return this->data[k];
	}
}

#endif
