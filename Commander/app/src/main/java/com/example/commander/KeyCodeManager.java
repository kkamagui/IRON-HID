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

import java.util.*;

/**
 *	Key code manager class
 *
 *	Translate key codes to ASCII and ASCII to key codes
 */
public class KeyCodeManager 
{
	byte [] mPrevKeyState = new byte [8];
	byte [] mCurKeyState = new byte [8];
	
	// Inverse map
	public HashMap<Object, String> mInverseUsbKeyMap = new HashMap<Object, String>();
	public HashMap<String, Object> mUsbKeyMap = new HashMap<String, Object>() 
	{{
		put("KEY_A", 4);
		put("KEY_B", 5);
		put("KEY_C", 6);
		put("KEY_D", 7);
		put("KEY_E", 8);
		put("KEY_F", 9);
		put("KEY_G", 10);
		put("KEY_H", 11);
		put("KEY_I", 12);
		put("KEY_J", 13);
		put("KEY_K", 14);
		put("KEY_L", 15);
		put("KEY_M", 16);
		put("KEY_N", 17);
		put("KEY_O", 18);
		put("KEY_P", 19);
		put("KEY_Q", 20);
		put("KEY_R", 21);
		put("KEY_S", 22);
		put("KEY_T", 23);
		put("KEY_U", 24);
		put("KEY_V", 25);
		put("KEY_W", 26);
		put("KEY_X", 27);
		put("KEY_Y", 28);
		put("KEY_Z", 29);
		put("KEY_1", 30);
		put("KEY_2", 31);
		put("KEY_3", 32);
		put("KEY_4", 33);
		put("KEY_5", 34);
		put("KEY_6", 35);
		put("KEY_7", 36);
		put("KEY_8", 37);
		put("KEY_9", 38);
		put("KEY_0", 39);
		put("KEY_ENTER", 40);
		put("KEY_ESC", 41);
		put("KEY_BACKSPACE", 42);
		put("KEY_TAB", 43);
		put("KEY_SPACE", 44);
		put("KEY_MINUS", 45);
		put("KEY_EQUAL", 46);
		put("KEY_LEFT_BRACE", 47);
		put("KEY_RIGHT_BRACE", 48);
		put("KEY_BACKSLASH", 49);
		put("KEY_SEMICOLON", 51);
		put("KEY_QUOTATION", 52);
		put("KEY_TILDE", 53);
		put("KEY_COMMA", 54);
		put("KEY_PERIOD", 55);
		put("KEY_SLASH", 56);
		put("KEY_CAPSLOCK", 57);
		put("KEY_F1", 58);
		put("KEY_F2", 59);
		put("KEY_F3", 60);
		put("KEY_F4", 61);
		put("KEY_F5", 62);
		put("KEY_F6", 63);
		put("KEY_F7", 64);
		put("KEY_F8", 65);
		put("KEY_F9", 66);
		put("KEY_F10", 67);
		put("KEY_F11", 68);
		put("KEY_F12", 69);
		put("KEY_PRINTSCREEN", 70);
		put("KEY_SCROLL_LOCK", 71);
		put("KEY_PAUSE", 72);
		put("KEY_INSERT", 73);
		put("KEY_HOME", 74);
		put("KEY_PAGEUP", 75);
		put("KEY_DELETE", 76);
		put("KEY_END", 77);
		put("KEY_PAGEDOWN", 78);
		put("KEY_RIGHT", 79);
		put("KEY_LEFT", 80);
		put("KEY_DOWN", 81);
		put("KEY_UP", 82);
		put("KEY_NUM_LOCK", 83);
		put("KEYPAD_SLASH", 84);
		put("KEYPAD_ASTERISK", 85);
		put("KEYPAD_MINUS", 86);
		put("KEYPAD_PLUS", 87);
		put("KEYPAD_ENTER", 88);
		put("KEYPAD_1", 89);
		put("KEYPAD_2", 90);
		put("KEYPAD_3", 91);
		put("KEYPAD_4", 92);
		put("KEYPAD_5", 93);
		put("KEYPAD_6", 94);
		put("KEYPAD_7", 95);
		put("KEYPAD_8", 96);
		put("KEYPAD_9", 97);
		put("KEYPAD_0", 98);
		put("KEYPAD_PERIOD", 99);
		put("KEY_APPLICATION", 101);
		put("KEY_POWER", 102);
		put("KEY_PAD_EQUAL", 103);
		put("KEY_F13", 104);
		put("KEY_F14", 105);
		put("KEY_F15", 106);
		put("KEY_F16", 107);
		put("KEY_F17", 108);
		put("KEY_F18", 109);
		put("KEY_F19", 110);
		put("KEY_F20", 111);
		put("KEY_F21", 112);
		put("KEY_F22", 113);
		put("KEY_F23", 114);
		put("KEY_F24", 115);
		put("KEY_EXECUTE", 116);
		put("KEY_HELP", 117);
		put("KEY_MENU", 118);
		put("KEY_SELECT", 119);
		put("KEY_STOP", 120);
		put("KEY_AGAIN", 121);
		put("KEY_UNDO", 122);
		put("KEY_CUT", 123);
		put("KEY_COPY", 124);
		put("KEY_PASTE", 125);
		put("KEY_FIND", 126);
		put("KEY_MUTE", 127);
		put("KEY_VOLUMEUP", 128);
		put("KEY_VOLUMEDOWN", 129);
		put("KEY_EJECT", 184);
		put("KEY_LCONTROL", 224);
		put("KEY_RSHIFT", 225);
		put("KEY_LALT", 226);
		put("KEY_LGUI", 227);
		put("KEY_RCONTROL", 228);
		put("KEY_LSHIFT", 229);
		put("KEY_RALT", 230);
		put("KEY_RGUI", 231);
	}};
	
	public HashMap<Object, String> mInverseConvertStirngMap = new HashMap<Object, String>();
	public HashMap<String, String> mConvertStringMap = new HashMap<String, String>() 
	{{
		put("a", "[KEY_A]");
		put("b", "[KEY_B]");
		put("c", "[KEY_C]");
		put("d", "[KEY_D]");
		put("e", "[KEY_E]");
		put("f", "[KEY_F]");
		put("g", "[KEY_G]");
		put("h", "[KEY_H]");
		put("i", "[KEY_I]");
		put("j", "[KEY_J]");
		put("k", "[KEY_K]");
		put("l", "[KEY_L]");
		put("m", "[KEY_M]");
		put("n", "[KEY_N]");
		put("o", "[KEY_O]");
		put("p", "[KEY_P]");
		put("q", "[KEY_Q]");
		put("r", "[KEY_R]");
		put("s", "[KEY_S]");
		put("t", "[KEY_T]");
		put("u", "[KEY_U]");
		put("v", "[KEY_V]");
		put("w", "[KEY_W]");
		put("x", "[KEY_X]");
		put("y", "[KEY_Y]");
		put("z", "[KEY_Z]");
		put("1", "[KEY_1]");
		put("2", "[KEY_2]");
		put("3", "[KEY_3]");
		put("4", "[KEY_4]");
		put("5", "[KEY_5]");
		put("6", "[KEY_6]");
		put("7", "[KEY_7]");
		put("8", "[KEY_8]");
		put("9", "[KEY_9]");
		put("0", "[KEY_0]");
		put(" ", "[KEY_SPACE]");
		put("-", "[KEY_MINUS]");
		put("=", "[KEY_EQUAL]");
		put("[", "[KEY_LEFT_BRACE]");
		put("]", "[KEY_RIGHT_BRACE]");
		put("\\", "[KEY_BACKSLASH]");
		put(";", "[KEY_SEMICOLON]");
		put("'", "[KEY_QUOTATION]");
		put("`", "[KEY_TILDE]");
		put(",", "[KEY_COMMA]");
		put(".", "[KEY_PERIOD]");
		put("/", "[KEY_SLASH]");
		put("A", "[KEY_LSHIFT][KEY_A]");
		put("B", "[KEY_LSHIFT][KEY_B]");
		put("C", "[KEY_LSHIFT][KEY_C]");
		put("D", "[KEY_LSHIFT][KEY_D]");
		put("E", "[KEY_LSHIFT][KEY_E]");
		put("F", "[KEY_LSHIFT][KEY_F]");
		put("G", "[KEY_LSHIFT][KEY_G]");
		put("H", "[KEY_LSHIFT][KEY_H]");
		put("I", "[KEY_LSHIFT][KEY_I]");
		put("J", "[KEY_LSHIFT][KEY_J]");
		put("K", "[KEY_LSHIFT][KEY_K]");
		put("L", "[KEY_LSHIFT][KEY_L]");
		put("M", "[KEY_LSHIFT][KEY_M]");
		put("N", "[KEY_LSHIFT][KEY_N]");
		put("O", "[KEY_LSHIFT][KEY_O]");
		put("P", "[KEY_LSHIFT][KEY_P]");
		put("Q", "[KEY_LSHIFT][KEY_Q]");
		put("R", "[KEY_LSHIFT][KEY_R]");
		put("S", "[KEY_LSHIFT][KEY_S]");
		put("T", "[KEY_LSHIFT][KEY_T]");
		put("U", "[KEY_LSHIFT][KEY_U]");
		put("V", "[KEY_LSHIFT][KEY_V]");
		put("W", "[KEY_LSHIFT][KEY_W]");
		put("X", "[KEY_LSHIFT][KEY_X]");
		put("Y", "[KEY_LSHIFT][KEY_Y]");
		put("Z", "[KEY_LSHIFT][KEY_Z]");
		put("!", "[KEY_LSHIFT][KEY_1]");
		put("@", "[KEY_LSHIFT][KEY_2]");
		put("#", "[KEY_LSHIFT][KEY_3]");
		put("$", "[KEY_LSHIFT][KEY_4]");
		put("%", "[KEY_LSHIFT][KEY_5]");
		put("^", "[KEY_LSHIFT][KEY_6]");
		put("&", "[KEY_LSHIFT][KEY_7]");
		put("*", "[KEY_LSHIFT][KEY_8]");
		put("(", "[KEY_LSHIFT][KEY_9]");
		put(")", "[KEY_LSHIFT][KEY_0]");
		put("_", "[KEY_LSHIFT][KEY_MINUS]");
		put("+", "[KEY_LSHIFT][KEY_EQUAL]");
		put("{", "[KEY_LSHIFT][KEY_LEFT_BRACE]");
		put("}", "[KEY_LSHIFT][KEY_RIGHT_BRACE]");
		put("|", "[KEY_LSHIFT][KEY_BACKSLASH]");
		put(":", "[KEY_LSHIFT][KEY_SEMICOLON]");
		put("\"", "[KEY_LSHIFT][KEY_QUOTATION]");
		put("~", "[KEY_LSHIFT][KEY_TILDE]");
		put("<", "[KEY_LSHIFT][KEY_COMMA]");
		put(">", "[KEY_LSHIFT][KEY_PERIOD]");
		put("?", "[KEY_LSHIFT][KEY_SLASH]");
	}};
	
	/**
	 *	Constructor
	 */
	public KeyCodeManager()
	{
		// Make inverse key map
        for (String key : mUsbKeyMap.keySet())
        {
        	Object keyCode = mUsbKeyMap.get(key);
            mInverseUsbKeyMap.put(keyCode, key);
        }
        
        for (String key : mConvertStringMap.keySet())
        {
        	Object keyString = mConvertStringMap.get(key);
        	mInverseConvertStirngMap.put(keyString, key);
        }
        mInverseConvertStirngMap.put("[KEY_ENTER]", "\n");
	}
	
	/**
	 *	Convert Ascii strings to special format
	 */
	public String ConvertAsciiKeyToKeyData(String input)
	{
		String convertedData = "";
		String[] spliteData = input.split("\n");

		for (String line : spliteData)
		{
			// if line is started with "[KEY_", skip this line 
			if (line.contains("[KEY_"))
			{
				convertedData += line + "\n";
				continue;
			}
			
			// Convert Ascii string to "[KEY_XXX]" format
			for (int i = 0 ; i < line.length() ; i++)
			{
				convertedData += mConvertStringMap.get(
					Character.toString(line.charAt(i))) + "\n";
			}
		}
		return convertedData;
	}

	/**
	 *	Convert special format to USB data format 
	 */
	public List<byte[]> ConvertKeyDataToUSBData(String input)
	{
		String[] spliteData = input.split("\n");
		List<byte[]> output = new ArrayList<byte[]>();
		byte [] delemeter = new byte [2];
		
		delemeter[0] = 0x00;
		delemeter[1] = (byte) 0xFF;

		for (String line : spliteData)
		{
			line = line.replace("[", "");
			line = line.replace("]", " ");

			String[] keys = line.split(" ");
			
			byte [] data = new byte [9];
			byte modifierByte = 0x00;
			int index = 2;
			
			// Convert each key to byte value
			for (String key : keys)
			{
				Integer keyCodeObject = (Integer) mUsbKeyMap.get(key);
				if (keyCodeObject == null)
				{
					continue;
				}

				int keyCode = (int) keyCodeObject;				
	
				// If key is modifier, Update first byte
				if (keyCode >= (Integer)mUsbKeyMap.get("KEY_LCONTROL"))
				{
					modifierByte |= 0x01 << (keyCode - 
						(Integer)mUsbKeyMap.get("KEY_LCONTROL"));
				}	
				else
				{
					data[index] = (byte)keyCode;
					index++;
					
					if (index >= 8)
					{
						break;
					}
				}
			}

			// Add key bytes with modifier & separator
			data[0] = modifierByte;
			data[index] = (byte) 0xFF;
			
			output.add(data);
			output.add(delemeter);
		}
		return output;
	}
	
	/**
	 *	Convert key monitor packet to string 
	 */
	public String ConvertKeyMonitorPacketToString(byte [] usbData)
	{
		String output = "";
		int key;
		int i;
		int j;
		
		System.arraycopy(usbData, 0, mCurKeyState, 0, 8);
		
		// Check previous pressed key
		//usbData[0] = (byte) ((usbData[0] ^ mPrevKeyState[0]) | usbData[0]);
		for (i = 2 ; i < 8 ; i++)
		{
			byte prevKey = mPrevKeyState[i];
			for (j = 2 ; j < 8 ; j++)
			{
				if (usbData[j] == prevKey)
				{
					usbData[j] = 0;
				}
			}
		}

		// Print key data
		for (i = 0 ; i < 8; i++)
		{
			key = usbData[i] & 0xFF;
			if (key == 0x00)
			{
				continue;
			}
			
			// Index 0 is modifier key
			if (i == 0)
			{
				for (j = 0 ; j < 8 ; j++)
				{
					if ((key & (0x01 << j)) != 0)
					{
						// KEY_LCONTROL
						output += "[" + mInverseUsbKeyMap.get(j + 224) + "]";	
					}
				}
			}
			else
			{
				output += "[" + mInverseUsbKeyMap.get(Integer.valueOf(key)) + "]";
			}
		}
		
		System.arraycopy(mCurKeyState, 0, mPrevKeyState, 0, 8);
		String convertedValue;
		convertedValue = output.replace("[KEY_RSHIFT]", "[KEY_LSHIFT]");
		convertedValue = mInverseConvertStirngMap.get(convertedValue);
		if (convertedValue != null)
		{
			output = convertedValue;
		}
		else
		{
			output = output.replace("[KEY_LSHIFT]", "");
			output = output.replace("[KEY_RSHIFT]", "");
			if (output.length() != 0)
			{
				output += "\n";
			}
		}
		return output;
	}
}
