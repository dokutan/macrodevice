/*
 * This sends notifications over serial when a pushbutton connected betwen Pin 10 and GND gets pressed or released.
 */

const int button_pin = 10;
int button_state_old = HIGH, button_state_new = HIGH;

void setup()
{
  Serial.begin(9600);
  pinMode( button_pin, INPUT_PULLUP );
}

void loop()
{

  // read button state
  button_state_old = button_state_new;
  button_state_new = digitalRead( button_pin );

  // button is pressed
  if( button_state_old == HIGH && button_state_new == LOW )
  {
    Serial.print( "pressed\n" ); // don't use println as that would end the line with \r\n
  }
  // button is released
  else if( button_state_old == LOW && button_state_new == HIGH )
  {
    Serial.print( "released\n" );
  }

  // wait for debouncing
  delay( 10 );
}
