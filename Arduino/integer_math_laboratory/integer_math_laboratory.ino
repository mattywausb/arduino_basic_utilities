#define VALUE_1_D20 0x100000
#define VALUE_1_D10 1024
#define D10_DECIMALS 10
#define D10_DECIMAL_MASK 0x03ff
#define VALUE_1_D7 128
#define D7_DECIMALS 7
#define SCALE_SHIFT 3

void setup() {
  // put your setup code here, to run once:
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
    Serial.setTimeout(100);
  
}

int16_t input_hue=180; //CYAN
int8_t input_saturation=100;
int8_t input_value=100;

int16_t input_fact=200;


void float_implementation()
{
  float f_input_hue=input_hue;
  float f_input_saturation=input_saturation/100.0;
  float f_input_value=input_value/100.0;

  float      hue_segment, p, partner_segment_fraction,hue_segment_fraction,white_factor,partner_factor;
  long        hue_segment_int;

  byte red,green,blue;

  Serial.println(F("----start: float implementation" ));
  // time critical rgb conversion startes here
  unsigned long start_rgb=micros();


  hue_segment = f_input_hue/60.0;
  hue_segment_int=(long) hue_segment;
  hue_segment_fraction=hue_segment-hue_segment_int;

  white_factor=f_input_value*(1.0-f_input_saturation);

  if(hue_segment_int%2==0) partner_segment_fraction = (1.0-(f_input_saturation * (1.0-hue_segment_fraction) ) );
  else                     partner_segment_fraction =  (1.0-(f_input_saturation *      hue_segment_fraction  ) );

  partner_factor = f_input_value * partner_segment_fraction;

  #define scale_float_to_255(f) f*255.0

  switch (hue_segment_int)
  {
     case 0:  // from red to yellow
            red=scale_float_to_255(f_input_value);
            green=scale_float_to_255(partner_factor);
            blue=scale_float_to_255(white_factor);
            break;
     case 1: // from yellow to green
            green=scale_float_to_255(f_input_value);
            red=scale_float_to_255(partner_factor);
            blue=scale_float_to_255(white_factor);
            break;
     case 2: // from green to cyan
            green=scale_float_to_255(f_input_value);
            blue=scale_float_to_255(partner_factor);
            red=scale_float_to_255(white_factor);
            break;
     case 3: // from cyan to blue
            blue=scale_float_to_255(f_input_value);
            green=scale_float_to_255(partner_factor);
            red=scale_float_to_255(white_factor);
            break;
     case 4: // from blue to magenta
            blue=scale_float_to_255(f_input_value);
            red=scale_float_to_255(partner_factor);
            green=scale_float_to_255(white_factor);
            break;
     case 5: // from magente to red
            red=scale_float_to_255(f_input_value);
            blue=scale_float_to_255(partner_factor);
            green=scale_float_to_255(white_factor);
            break;

   } // switch
  
  // end of time critical part
  unsigned long end_time = micros();

  // Print results
  Serial.print(F("Runtime rgb: ") );Serial.print(end_time-start_rgb);Serial.println(F(" us") );
  Serial.print(F("f_input_hue= "));Serial.println(f_input_hue);
  Serial.print(F("f_input_saturation= "));Serial.println(f_input_saturation);
  Serial.print(F("f_input_value= "));Serial.println(f_input_value);
  Serial.print(F("hue_segment_fraction= "));Serial.println(hue_segment_fraction);
  Serial.print(F("white_factor= "));Serial.println(white_factor);
  Serial.print(F("partner_segment_fraction= "));Serial.println(partner_segment_fraction);
  Serial.print(F("partner_factor= "));Serial.println(partner_factor);
  Serial.print(F("rgb = ("));Serial.print(red);
  Serial.print(F(","));Serial.print(green);
  Serial.print(F(","));Serial.print(blue);Serial.println(F(")"));

}

void integer_implementation()
{

  byte red,green,blue;
Serial.println(F("----start: Integer implementation" ));
  unsigned long start_time = micros();
  unsigned long start_rgb =0;
  int32_t partner_segment_fraction_d20;
  // multiply in binary fixed point artihmetic 128=1.00

  uint8_t value_d7=((int16_t)(input_value<<D7_DECIMALS))/100;  // convert from 0-100 to 0-128 (100=128)
                                                          // useful wenn 100 is fixed integer with 2 decimals (1.00)
                                                          // this results in a binary number with 7 "decimals" (1.0000000)
  int16_t fact_d7=((input_fact<<7))/100;

  uint8_t product_d7=((value_d7*fact_d7)>>D7_DECIMALS);   // Muliply while having 7 binary "decimals"
  int8_t  output_product=(((int16_t)product_d7 *100)>>D7_DECIMALS);

  uint8_t saturation_d7=((int16_t)(input_saturation<<D7_DECIMALS))/100;  // Convert from 3 digit Decimal to 8 Digit binary 

  int16_t hue_d10=((input_hue*17)+((input_hue/15))); // convert from 360 = 6144 or 60=1024, allowing fast division by 60 for rgb calculation
                                                        // since 100 0000 0000 is equivalent to 60 = resolution of 0,06 degree

  // time critical rgb conversion startes here
  start_rgb=micros();
  int8_t hue_segment=hue_d10>>D10_DECIMALS;   // "Divide hue degrees by 60" (6 Segments red - yellow- green- cyan - blue - magenta)
  int16_t hue_segment_fraction_d10=hue_d10 & D10_DECIMAL_MASK; //Get last 10 Bits = "Modulo 60" = fraction inside in the segment
  int16_t white_factor_d7=((int16_t)value_d7*((int16_t)VALUE_1_D7-(int16_t)saturation_d7))>>D7_DECIMALS; // >>7 for Decimal point shifting

  if(hue_segment%2==0)  partner_segment_fraction_d20 = ((int32_t)VALUE_1_D20 - (((int32_t)saturation_d7)<<SCALE_SHIFT) * (VALUE_1_D10-hue_segment_fraction_d10) );
  else                  partner_segment_fraction_d20 = ((int32_t)VALUE_1_D20 - (((int32_t)saturation_d7)<<SCALE_SHIFT) *               hue_segment_fraction_d10  ) ;

  int16_t partner_factor_d10= ((((int32_t)value_d7)<<SCALE_SHIFT) *partner_segment_fraction_d20)>>(2*D10_DECIMALS);

  #define scale_d7_to_255(b) b==VALUE_1_D7?255:b<<1
  #define scale_d10_to_255(b) b>=VALUE_1_D10?255:b>>2

  switch (hue_segment)
  {
     case 0:  // from red to yellow
            red=scale_d7_to_255(value_d7);
            green=scale_d10_to_255(partner_factor_d10);
            blue=scale_d7_to_255(white_factor_d7);
            break;
     case 1: // from yellow to green
            green=scale_d7_to_255(value_d7);
            red=scale_d10_to_255(partner_factor_d10);
            blue=scale_d7_to_255(white_factor_d7);
            break;
     case 2: // from green to cyan
            green=scale_d7_to_255(value_d7);
            blue=scale_d10_to_255(partner_factor_d10);
            red=scale_d7_to_255(white_factor_d7);
            break;
     case 3: // from cyan to blue
            blue=scale_d7_to_255(value_d7);
            green=scale_d10_to_255(partner_factor_d10);
            red=scale_d7_to_255(white_factor_d7);
            break;
     case 4: // from blue to magenta
            blue=scale_d7_to_255(value_d7);
            red=scale_d10_to_255(partner_factor_d10);
            green=scale_d7_to_255(white_factor_d7);
            break;
     case 5: // from magente to red
            red=scale_d7_to_255(value_d7);
            blue=scale_d10_to_255(partner_factor_d10);
            green=scale_d7_to_255(white_factor_d7);
            break;

   } // switch

  // end of time critical part
  unsigned long end_time = micros();

  // Print results
  Serial.print(F("Runtime: " ));Serial.print(end_time-start_time);Serial.println(F(" us" ));
  Serial.print(F("Runtime rgb: " ));Serial.print(end_time-start_rgb);Serial.println(F(" us") );

  Serial.print(F("input_hue= "));Serial.println(input_hue);
  Serial.print(F("input_saturation= "));Serial.println(input_saturation);
  Serial.print(F("input_value= "));Serial.println(input_value);
  Serial.print(F("hue_d10= "));Serial.print(hue_d10);Serial.print(F(" 0x"));Serial.println(hue_d10,HEX);
  Serial.print(F("saturation_d7= "));Serial.print(saturation_d7);Serial.print(F(" 0x"));Serial.println(saturation_d7,HEX);
  Serial.print(F("value_d7= "));Serial.print(value_d7);Serial.print(F(" 0x"));Serial.println(value_d7,HEX);
  Serial.print(F("fact_d7= "));Serial.print(fact_d7);;Serial.print(F(" 0x"));Serial.println(fact_d7,HEX);
  Serial.print(F("product_d7= "));Serial.print(product_d7); Serial.print(F(" 0x"));Serial.println(product_d7,HEX);
  Serial.print(F("hue_segment= "));Serial.println(hue_segment); 
  Serial.print(F("hue_segment_fraction_d10= "));Serial.print(hue_segment_fraction_d10); Serial.print(F(" 0x"));Serial.println(hue_segment_fraction_d10,HEX);
  Serial.print(F("white_factor_d7= "));Serial.print(white_factor_d7); Serial.print(F(" 0x"));Serial.println(white_factor_d7,HEX);
  Serial.print(F("partner_segment_fraction_d20= "));Serial.print(partner_segment_fraction_d20); Serial.print(F(" 0x"));Serial.println(partner_segment_fraction_d20,HEX);
  Serial.print(F("partner_factor_d10= "));Serial.print(partner_factor_d10); Serial.print(F(" 0x"));Serial.println(partner_factor_d10,HEX);
  
  Serial.print(F("rgb = ("));Serial.print(red);
  Serial.print(F(","));Serial.print(green);
  Serial.print(F(","));Serial.print(blue);Serial.println(F(")"));
  Serial.println();
  Serial.print(F("input_fact= "));Serial.println(input_fact);
  Serial.print(F("output_product ="));Serial.println(output_product);


  
}

void loop() {
  int serial_number_input=0;
  int serial_char_input=0;
  if(Serial.available() > 0) {
    serial_char_input=Serial.read();
    switch(serial_char_input) {
      case 'h': input_hue=Serial.parseInt(); break;
      case 's': input_saturation=Serial.parseInt(); break;
      case 'v': input_value=Serial.parseInt(); break;
    } //switch
    integer_implementation();
    Serial.println(F(". . . . . . . . . . . . " ));
    float_implementation();
    Serial.println(F("----------------------" ));
  }

}
