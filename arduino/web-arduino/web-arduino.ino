#include <MIDI.h>

const int PINS=80;
const int A_VAL_MIN=0;
const int A_VAL_MAX=0;

byte CONFIG_ANALOG_PIN[PINS]; // 0: notconfigured, NoteOff: output, NoteOn: input
byte CONFIG_DIGITAL_PIN[PINS]; // 0: notconfigured, NoteOff: output, NoteOn: input

int VALUE_DIGITAL_PIN[PINS];
int VALUE_ANALOG_PIN[PINS];

int CONFIG_ANALOG_VAL_MIN[PINS];
int CONFIG_ANALOG_VAL_MAX[PINS];

MIDI_CREATE_DEFAULT_INSTANCE();

void setup()
{
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  //initialize config
  initialize_config();

/*
  // lcd test
  i2c_lcd_setup();
  lcd_setCursor(0, 0);
  byte a=0x80;
  char text[80];
  sprintf(text, "%c", a);
  lcd_printStr(text);
  lcd_setCursor(1, 1);
  lcd_printStr("SCIENCE");
*/
}

void initialize_config() {
  for(int i=0; i<PINS; i++) {
    CONFIG_ANALOG_PIN[i]=0;
    CONFIG_ANALOG_VAL_MIN[i]=A_VAL_MIN;
    CONFIG_ANALOG_VAL_MAX[i]=A_VAL_MAX;
  }

  for(int i=0; i<PINS; i++) {
    CONFIG_DIGITAL_PIN[i]=0;
  }
}


void loop()
{
  // watch input 
  if (MIDI.read()) {
    byte type = MIDI.getType();
    int channel = MIDI.getChannel();
    int note = MIDI.getData1();
    int velocity = MIDI.getData2();

    if(type==midi::ControlChange && note==0x78) {
      initialize_config();
    }

    switch(channel) {
      case 16: // digital pin
        if(type==midi::NoteOff){
          CONFIG_DIGITAL_PIN[note]=type;
          pinMode(note, OUTPUT);
        }
        if(type==midi::NoteOn) {
          CONFIG_DIGITAL_PIN[note]=type;
          pinMode(note, INPUT);
        }
        break;
      case 15: // analog pin
        // for analog
        if(type==midi::NoteOff){
          CONFIG_ANALOG_PIN[note]=type;
          //pinMode(note, OUTPUT);
        }
        if(type==midi::NoteOn) {
          CONFIG_ANALOG_PIN[note]=type;
          //pinMode(note, INPUT);
        }
        break; 
      case 14: // analog config value MAX 
        if(type==midi::NoteOff){
          CONFIG_ANALOG_VAL_MAX[note]+=velocity*128;
        }
        if(type==midi::NoteOn) {
          CONFIG_ANALOG_VAL_MAX[note]+=velocity;
        }
        break; 
      case 13: // analog config value MIX 
        if(type==midi::NoteOff){
          CONFIG_ANALOG_VAL_MIN[note]+=velocity*128;
        }
        if(type==midi::NoteOn) {
          CONFIG_ANALOG_VAL_MIN[note]+=velocity;
        }
        break; 
      case 3:
        if (type==midi::NoteOff) {
          digitalWrite(note, LOW);
        }
        if (type==midi::NoteOn) {
          digitalWrite(note, HIGH);
        }
        break;
      default:
        break;
    }
  }

  // watch digital output 
  for(int i=0; i<PINS; i++) {
    if(CONFIG_DIGITAL_PIN[i]==midi::NoteOn) {
      int val=digitalRead(i);
      if(VALUE_DIGITAL_PIN[i]!=val) {
        if(val==LOW) {
          MIDI.sendNoteOff(i, 0, 3);
        } else 
        if(val==HIGH) {
          MIDI.sendNoteOn(i, 0, 3);
        }
        VALUE_DIGITAL_PIN[i]=val;
      }
    }
  }

  // watch analog output 
  for(int i=0; i<PINS; i++) {
    if(CONFIG_ANALOG_PIN[i]==midi::NoteOn) {
      int val=map(analogRead(i), CONFIG_ANALOG_VAL_MIN[i], CONFIG_ANALOG_VAL_MAX[i], 0, 127);
      if(abs(VALUE_ANALOG_PIN[i]-val)>2) {
        if(val==0) {
          MIDI.sendNoteOff(i, 0, 1);
        } else { 
          MIDI.sendNoteOn(i, val, 1);
        }
        VALUE_ANALOG_PIN[i]=val;
      }
    }
  }

}


