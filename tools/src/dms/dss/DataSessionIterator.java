package dms.dss;

import java.util.*;
import dms.dss.*;

/**
 * Description: Implements the standard list interface.
 *
 * We maintain the concept of the ListIterator.  That is, there is no
 * current element. The index member variable is treated as lying between elements.
 * Rather than actually doing this, the index below represents the next
 * element. Thus when we call next(), we return the element at index
 * BEFORE we increment. A call to previous returns the element at index
 * AFTER decrementing index. Thus, alternating calls to next and previous
 * return the same element. As required by the ListIterator specification.
 *
 * <P>CVS $Id: DataSessionIterator.java,v 1.5 2003/08/27 17:07:38 khuck Exp $</P>
 * @author	Kevin Huck, Robert Bell
 * @version	0.1
 */
public class DataSessionIterator implements ListIterator{

/**
 * Standard Constructor for the DataSessionIterator class.
 *
 * @param	inVector Vector object to be converted to an Iterator
 */
		public DataSessionIterator(Vector inVector){
				
				//Check to make sure that the Vector is not null.
				if(inVector == null)
						throw new IllegalArgumentException();
				
				//Safe to continue.				
				listData = inVector;
				size = listData.size();
		}


		//List Iterator implemetation. Additional methods follow afterwards.

		public void add(Object o){
				throw new UnsupportedOperationException();
		}

		public boolean hasNext(){
				if(index < size)
						return true;
				else
						return false;
		}

		public boolean hasPrevious(){
				if(index == 0)
						return false;
				else
						return true;
		}

		public Object next(){
				
				Object nextObject = null;

				//Check to make sure we are not over the end.
				if(index >= size){
						throw new NoSuchElementException();
				}
				else{
						//Since, by the specification, alternating calls to next and previous
						//return the same element. We get the element first, and then increment
						//the index.  The reverse of previous(). See instance notes below.
						nextObject = listData.elementAt(index);
						
						//Increment the index.
						index++;
				}
				
				return nextObject;
		}

		public int nextIndex(){
				return index++;
		}

		public Object previous(){

				Object previousObject = null;

				if(index == 0){
						throw new NoSuchElementException();
				}
				else{
						//Since, by the specification, alternating calls to next and previous
						//return the same element. We decrement first, and then return.  The
						//reverse of next(). See instance notes below.
						index--;
						previousObject =  listData.elementAt(index);
				}

				return previousObject;
		}

		public int previousIndex(){
				return index--;
		}

		public void remove(){
				throw new UnsupportedOperationException();
		}

		public void set(Object o){
				throw new UnsupportedOperationException();
		}


		//Methods in addition to the standard ListIterator.

		//Resets the Iterator to its initial state.
/**
 * Resets the Iterator to its initial state.
 *
 */
		public void reset(){
				index = 0;
		}

/**
 * Gives the number of elements this iterator maintains.
 *
 * @return int number of elements in the iterator.
 */
		public int size(){
				return size;
		}


		private Vector listData = null;
		private int size = 0;
		private int index = 0;
}
