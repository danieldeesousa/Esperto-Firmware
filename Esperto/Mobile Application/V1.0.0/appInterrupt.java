import app;

public class EspertoApp
	{
		private String command_string = "date|time|phone|text\n"; // data packet
		private bool isPhone = false; // is phone ringinig
		private String whoPhone = ""; // who is calling
		private int stayCount = 0; // seconds which phone notification has been on
		private bool isText = false; // is there a new text
		private String whoText = ""; // who texted
		private int stayCount2; // seconds which text notification has been on
		
		// called when app is initialized
		public void init()
		{
			// if bluetooth is on, set status
			if(bluetoothMaster.isEnabled == true)
				app.ConnectedStatus = 'Not Connected';
			
			// get addresses and names of bluetooth devices
			bluetoothConnect.Elements = bluetoothMaster.AddressesAndNames;
		}
		
		// called when connect button is pressed
		public void connectButtonPress()
		{
			// connect to the device which is selected
			bluetoothMaster.Connect(bluetoothConnect.Selection);
			app.ConnectedStatus = 'Connected';
		}
		
		// interrupt calls function every second timed by clock
		public void sendData()
		{
			// get time and date from phone clock
			command_string.time = Clock1.time();
			command_string.date = Clock1.date();
			
			// if there is a call notification
			if(isPhone == true)
			{
				
				// if we have reached end of notification, stop
				if(stayCount == 4)
				{
					stayCount = 0;
					isPhone = false;
				}
				
				// set the call data
				else
				{
					command_string.phone = whoPhone;
					stayCount++;
				}
			}
			
			// if there is a text notification
			if(isText == true)
			{
				
				// if we have reached end of notification, stop
				if(stayCount2 == 4)
				{
					stayCount2 = 0;
					isText = false;
				}
				
				// set the text data
				else
				{
					command_string.text = whoText;
					stayCount++;
				}
			}
			
			// send the data via bluetooth
			bluetoothMaster.send(command_string);
			
			// reset data packet
			command_string = "date|time|phone|text\n";
		}
		
		// interrupt calls this function when a call is received
		public void incomingCall()
		{
			if(app.status == 1)
			{
				isPhone = true;
				whoPhone = app.PhoneNumber;
			}
		}
		
		// interrupt calls this function when a text is received
		public void incomingText()
		{
			isText = true;
			whoText = app.number;
		}
	}
}