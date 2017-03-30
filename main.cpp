#include "mbed.h"
#include "utils.hpp"

#include "EthernetInterface.h"
#include "frdm_client.hpp"

#include "metronome.hpp"

#define IOT_ENABLED
#define play true
#define learn false 
Serial pc(USBTX,USBRX);
DigitalOut g_led_red(LED1);
DigitalOut g_led_green(LED2);
DigitalOut g_led_blue(LED3);
namespace active_low
{
	const bool on = false;
	const bool off = true;
}

InterruptIn g_button_mode(SW3);
InterruptIn g_button_tap(SW2);
bool mode=true;
bool fboardsetval=false;
size_t count=0;
metronome* met = new metronome();
int flag=0;
int val=0;
bool isPutCalled = false;

int maxBPM=0;
int minBPM=65535;

M2MResource* bpm_res;
M2MResource* min_res;
M2MResource* max_res;
M2MResource* reset_res;


void changeVal(const char*){
        isPutCalled=true;
}

void reset(void *argument)
 {
   min_res->set_value(0);
   max_res->set_value(0);
   maxBPM=0;
   minBPM=65535;
 }
    
void on_mode()
{
    
    // Change modes
    mode=!mode;
    if(mode==learn)
    	{
    		g_led_green = active_low::off;
    		//met.start_timing();	
    	}	
    else
    	{
    	if(count>=4)
    	(*met).set_bpmboard();
    	fboardsetval=true;
    	count=0;
       	(*met).stop_timing();
       	(*met).init();
       	flag=0;
    	}	    
}

void on_tap()
{
    // Receive a tempo tap
    if(mode==play)
    return;
    else{
    	count++;
    	if(flag==0)
    		{
    			g_led_red = active_low::on;
    			wait(0.2f);
	    		g_led_red = active_low::off;
    			(*met).start_timing();
    			flag=1;
    		}
    	else
    	{	
    	g_led_red = active_low::on;
    	wait(0.2f);
    	g_led_red = active_low::off;
    	(*met).tap(); 
    	}   		
    }
}

int main()
{
	// Seed the RNG for networking purposes
    unsigned seed = utils::entropy_seed();
    srand(seed);

	// LEDs are active LOW - true/1 means off, false/0 means on
	// Use the constants for easier reading
    g_led_red = active_low::off;
    g_led_green = active_low::off;
    g_led_blue = active_low::off;

	// Button falling edge is on push (rising is on release)
    g_button_mode.fall(&on_mode);
    g_button_tap.fall(&on_tap);
#ifdef IOT_ENABLED
	// Turn on the blue LED until connected to the network
    //g_led_blue = active_low::off;

	// Need to be connected with Ethernet cable for success
    EthernetInterface ethernet;
    if (ethernet.connect() != 0)
        return 1;

	// Pair with the device connector
    frdm_client client("coap://api.connector.mbed.com:5684", &ethernet);
    if (client.get_state() == frdm_client::state::error)
        return 1;


	// added by harsh
	
	 	// create ObjectID with metadata tag of '3201', which is 'digital output'
        M2MObject* bpm_object = M2MInterfaceFactory::create_object("3318");
        M2MObjectInstance* bpm_inst = bpm_object->create_object_instance();

        // 5853 = Multi-state output
        bpm_res = bpm_inst->create_dynamic_resource("5900", "BPM", M2MResourceInstance::STRING, false);
        
        bpm_res->set_operation(M2MBase::GET_PUT_ALLOWED);
        bpm_res->set_value((const uint8_t*)"0", 1);
		bpm_res->set_value_updated_function(value_updated_callback(&changeVal));//get        
		
		min_res = bpm_inst->create_dynamic_resource("5601", "MIN", M2MResourceInstance::STRING, false);
        
        min_res->set_operation(M2MBase::GET_ALLOWED);
        min_res->set_value((const uint8_t*)"0", 1);   
        
        //code for max.
        max_res = bpm_inst->create_dynamic_resource("5602", "MAX", M2MResourceInstance::STRING, false);
        
        max_res->set_operation(M2MBase::GET_ALLOWED);
        max_res->set_value((const uint8_t*)"0", 1); 
        
        //code for reset.
        reset_res = bpm_inst->create_dynamic_resource("5605", "RESET", M2MResourceInstance::STRING, false);
        
        reset_res->set_operation(M2MBase::POST_ALLOWED); 
        reset_res->set_execute_function(execute_callback(&reset));


	// The REST endpoints for this device
	// Add your own M2MObjects to this list with push_back before client.connect()
    M2MObjectList objects;

    M2MDevice* device = frdm_client::make_device();
    objects.push_back(device);
	objects.push_back(bpm_object);//added by me

	// Publish the RESTful endpoints
    client.connect(objects);
    
	// Connect complete; turn off blue LED forever
    //g_led_blue = active_low::on;
#endif

    while (true)
    {
#ifdef IOT_ENABLED
        if (client.get_state() == frdm_client::state::error)
            break;
          
        if(isPutCalled){
        	uint8_t* buffIn = NULL;
        	uint32_t sizeIn;  	
        	bpm_res->get_value(buffIn, sizeIn);
        	int x = atoi((char*)buffIn);
        	(*met).setbpm_cloud(x);
        	isPutCalled = false;   
        	if(x>maxBPM)
        	{
        		maxBPM = x;
        		max_res->set_value(maxBPM);
        	} 
        	if(x<minBPM)
        	{
        		minBPM = x;
        		min_res->set_value(minBPM);
        	}  	     	
        }
        if(fboardsetval){        	
        	bpm_res->set_value((*met).getbpm());
        	fboardsetval=false;
        	int currBPM = (*met).getbpm();
        	if(currBPM>maxBPM)
        	{
        		maxBPM = currBPM;
        		max_res->set_value(maxBPM);
        	} 
        	if(currBPM<minBPM)
        	{
        		minBPM = currBPM;
        		min_res->set_value(minBPM);
        	} 
        }	    
#endif
		
		if(mode==play&& (*met).getbpm()>0){
			
			g_led_green = active_low::on;
			wait(0.2f); 
			g_led_green = active_low::off;
			wait((60.0/(*met).getbpm())-0.2f);
    	  	
    	  }
    
        // Insert any code that must be run continuously here
    }

#ifdef IOT_ENABLED
    client.disconnect();
#endif

    return 1;
}