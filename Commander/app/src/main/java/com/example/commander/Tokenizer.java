/*
           IRON-HID Commander App for the Android Smartphone       
                    Copyright (C) 2016 Seunghun Han 
         at National Security Research Institute of South Korea
*/

/*
Copyright (c) 2016 Seunghun Han at NSR of South Kora

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

package com.example.commander;

/**
 * 	Tokenizer for the bluetooth serial
 */
public class Tokenizer 
{
	byte [] mBuffer;
	int mBufferSize = 1024 * 1024;
	int mBufferIndex;
	
	/**
	 *	Constructor
	 */
	public Tokenizer()
	{
		mBuffer = new byte [mBufferSize];
		mBufferIndex = 0;
	}
	
	/**
	 *	Upgrade buffer size
	 */
	public void UpgradeBufferSize()
	{
		byte [] tempBuffer = new byte [mBufferSize * 2];
		
		System.arraycopy(mBuffer, 0, tempBuffer, 0, mBufferSize);
		mBuffer = null;
		mBuffer = tempBuffer;
		mBufferSize *= mBufferSize;
	}
	
	/**
	 *	Add data to buffer
	 */
	public void AddData(byte [] data, int size)
	{	
		if (mBufferIndex + size > mBufferSize)
		{
			UpgradeBufferSize();
		}
		
		System.arraycopy(data, 0, mBuffer, mBufferIndex, size);
		mBufferIndex += size;
	}
	
	/**
	 *	Get token from buffer
	 */
	public int GetToken(int fixedTokenSize, byte [] result)
	{
		//byte [] result = null;
		int i = 0;
		int tokenSize = -1;

		if (fixedTokenSize != 0)
		{
			if (mBufferIndex > fixedTokenSize)
			{
				//result = new byte [fixedTokenSize];
				System.arraycopy(mBuffer, 0, result, 0, fixedTokenSize);
				System.arraycopy(mBuffer, fixedTokenSize + 1, mBuffer, 0, 
				mBufferIndex - fixedTokenSize - 1);
				mBufferIndex -= fixedTokenSize + 1;
				tokenSize = fixedTokenSize;
			}
		}
		else
		{
			for (i = 0 ; i < mBufferIndex ; i++)
			{
				if (mBuffer[i] == ';')
				{
					//result = new byte [i];
					System.arraycopy(mBuffer, 0, result, 0, i);
					System.arraycopy(mBuffer, i + 1, mBuffer, 0, mBufferIndex - 
						i - 1);
					mBufferIndex -= (i + 1);
					tokenSize = i;
					break;
				}
			}
		}
		
		return tokenSize; 
	}
	
	/**
	 *	Clear buffer
	 */
	public void ClearBuffer()
	{
		mBufferIndex = 0;
	}
}
