#ifndef __MATRIX_CPP__
#define __MATRIX_CPP__

#include <vector>
#include <cassert>

using namespace std;

namespace GenericClassLib {
	const int DEFAULT_MATRIX_SIZE = 4;

	template <typename itemType> class matrix {
	public:
		// constructors/destructor
		matrix(); // default size 4x4
		matrix(int rows,int cols); // size rows x cols
		matrix(int rows,int cols,const itemType & fillValue); // all entries == fillValue

		~matrix(); // destructor

		// assignment
		const matrix& operator =(const matrix & rhs);

		// accessors
		int numrows(void) const;                             // number of rows
		int numcols(void) const;                             // number of columns

		// indexing
		const vector<itemType>& operator [] (int k) const;  // range-checked indexing
		vector<itemType>& operator [] (int k);              // range-checked indexing

		// modifiers
		void resize(int newRows,int newCols);   // resizes matrix to newRows x newCols

	private:
		int myRows;                             // # of rows (capacity)
		int myCols;                             // # of cols (capacity)
		vector<vector<itemType> > myMatrix; // the matrix of items
	};
	
	template <typename itemType>
	matrix<itemType>::matrix()
		: myRows(DEFAULT_MATRIX_SIZE), myCols(DEFAULT_MATRIX_SIZE), myMatrix(DEFAULT_MATRIX_SIZE) {
		// postcondition: matrix of size 4x4 is constructed
		for(int k = 0; k < DEFAULT_MATRIX_SIZE; k++)
			myMatrix[k].resize(DEFAULT_MATRIX_SIZE);
	}

	template <typename itemType>
	matrix<itemType>::matrix(int rows,int cols)
		: myRows(rows), myCols(cols), myMatrix(rows) {
    	// precondition: 0 <= rows and 0 <= cols
		// postcondition: matrix of size rows x cols is constructed
		for(int k = 0; k < rows; k++)
			myMatrix[k].resize(cols);
	}

	template <typename itemType>
	matrix<itemType>::matrix(int rows, int cols, const itemType & fillValue)
        : myRows(rows), myCols(cols), myMatrix(rows) {
     
		// precondition: 0 <= rows and 0 <= cols
		// postcondition: matrix of size rows x cols is constructed
		//                all entries are set by assignment to fillValue after
		//                default construction
		//     
		for(int j = 0; j < rows; j++) {
			myMatrix[j].resize(cols);
			for(int k = 0; k < cols; k++)
				myMatrix[j][k] = fillValue;
		}
	}

	template <typename itemType>
	matrix<itemType>::~matrix () {
		// postcondition: matrix is destroyed
		// vector destructor frees everything
	}

	template <typename itemType>
	const matrix<itemType> &
	matrix<itemType>::operator =(const matrix<itemType>& rhs) {
		// postcondition: normal assignment via copying has been performed
		//                (if matrix and rhs were different sizes, matrix has 
		//                been resized to match the size of rhs)     
		if (this != &rhs) {                  // don't assign to self!
			myMatrix.resize(rhs.myRows);     // resize to proper # of rows
			myRows = rhs.myRows;             // set dimensions
			myCols = rhs.myCols;
        
			// copy rhs
			for(int k = 0; k < myRows; k++)
				myMatrix[k] = rhs.myMatrix[k];
		}
		return *this;       
	}

	template <typename itemType>
	int matrix<itemType>::numrows() const {
		// postcondition: returns number of rows
		return myRows;
	}

	template <typename itemType>
	int matrix<itemType>::numcols() const {
		// postcondition: returns number of columns
		return myCols;
	}


	template <typename itemType>
	void matrix<itemType>::resize(int newRows, int newCols) {
		// precondition: matrix size is rows X cols,
		//	             0 <= newRows and 0 <= newCols
		// postcondition: matrix size is newRows X newCols;
		//                for each 0 <= j <= min(rows,newRows) and
		//                for each 0 <= k <= min(cols,newCols), matrix[j][k] is
		//                a copy of the original; other elements of matrix are
		//                initialized using the default constructor for itemType
		//                Note: if newRows < rows or newCols < cols,
		//                      elements may be lost
		//
		myMatrix.resize(newRows);

		for(int k=0; k < newRows; k++)
			myMatrix[k].resize(newCols);
    
		myRows = newRows;
		myCols = newCols;
	}

	template <typename itemType>
	const vector<itemType> & 
	matrix<itemType>::operator [] (int k) const {
		// precondition: 0 <= k < number of rows
		// postcondition: returns k-th row
		assert(0 <= k);
		assert(k < myRows);
		return myMatrix[k];
	}

	template <class itemType>
	vector<itemType>& matrix<itemType>::operator [](int k) {
		// precondition: 0 <= k < number of rows
		// postcondition: returns k-th row
		assert(k < myRows);
		assert(0 <= k);
		return myMatrix[k];
	}
}

#endif
