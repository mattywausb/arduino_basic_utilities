#define SEGMENT_60_EQUIVALENT 2048
#define SEGMENT_BINARY_DECIMALS 11
#define SEGMENT_DECIMAL_MASK 0x07ff
#define VALUE_1_EQUIVALENT 128
#define VALUE_BINARY_DECIMALS 7

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

void integer_implementation()
{
Serial.println("-start- Integer implementation" );
  unsigned long start_time = micros();
  unsigned long start_rgb =0;
  // multiply in binary fixed point artihmetic 128=1.00

  uint8_t int8_value=((int16_t)(input_value<<VALUE_BINARY_DECIMALS))/100;  // convert from 0-100 to 0-128 (100=128)
                                                          // useful wenn 100 is fixed integer with 2 decimals (1.00)
                                                          // this results in a binary number with 7 "decimals" (1.0000000)
  int16_t int16_fact=((input_fact<<7))/100;

  uint8_t int8_product=((int8_value*int16_fact)>>VALUE_BINARY_DECIMALS);   // Muliply while having 7 binary "decimals"
  int8_t  output_product=(((int16_t)int8_product *100)>>VALUE_BINARY_DECIMALS);

  uint8_t int8_saturation=((int16_t)(input_saturation<<VALUE_BINARY_DECIMALS))/100;  // Convert from 3 digit Decimal to 8 Digit binary 

  int16_t int16_hue=((input_hue*34)+((input_hue/15)<<1)); // convert from 360 = 12288 or 60=2048, allowing fast division by 60 for rgb calculation

  start_rgb=micros();
  int8_t hue_segment=int16_hue>>SEGMENT_BINARY_DECIMALS;   // Divide by "60"
  int16_t hue_segment_offset=int16_hue & SEGMENT_DECIMAL_MASK; //Get last 11 Bits = "Modulo 60"
  int16_t int16_white_factor=(int8_value*(VALUE_1_EQUIVALENT-int8_saturation))>>VALUE_BINARY_DECIMALS; // >>7 for Decimal point shifting

  int16_t int16_growing_neighbor=((int16_t)int8_value*(VALUE_1_EQUIVALENT-((int8_saturation*(hue_segment_offset)>>4)) >> VALUE_BINARY_DECIMALS))>>VALUE_BINARY_DECIMALS;
  int16_t int16_fading_neighbor=((int16_t)int8_value*(VALUE_1_EQUIVALENT-((int8_saturation*((SEGMENT_60_EQUIVALENT-hue_segment_offset)>>4)) >> VALUE_BINARY_DECIMALS)))>>VALUE_BINARY_DECIMALS;



  // Print results
  unsigned long end_time = micros();
  Serial.print("Runtime: " );Serial.print(end_time-start_time);Serial.println(" us" );
  Serial.print("Runtime rgb: " );Serial.print(end_time-start_rgb);Serial.println(" us" );

  Serial.print("input_hue= ");Serial.println(input_hue);
  Serial.print("input_saturation= ");Serial.println(input_saturation);
  Serial.print("input_value= ");Serial.println(input_value);
  Serial.print("input_fact= ");Serial.println(input_fact);
  Serial.print("int16_hue= ");Serial.print(int16_hue);Serial.print(" 0x");Serial.println(int16_hue,HEX);
  Serial.print("int8_saturation= ");Serial.print(int8_saturation);Serial.print(" 0x");Serial.println(int8_saturation,HEX);
  Serial.print("int8_value= ");Serial.print(int8_value);Serial.print(" 0x");Serial.println(int8_value,HEX);
  Serial.print("int16_fact= ");Serial.print(int16_fact);;Serial.print(" 0x");Serial.println(int16_fact,HEX);
  Serial.print("int8_product= ");Serial.print(int8_product); Serial.print(" 0x");Serial.println(int8_product,HEX);
  Serial.print("hue_segment= ");Serial.print(hue_segment); Serial.print(" 0x");Serial.println(hue_segment,HEX);
  Serial.print("hue_segment_offset= ");Serial.print(hue_segment_offset); Serial.print(" 0x");Serial.println(hue_segment_offset,HEX);
  Serial.print("int16_white_factor= ");Serial.print(int16_white_factor); Serial.print(" 0x");Serial.println(int16_white_factor,HEX);
  Serial.print("int16_growing_neighbor= ");Serial.print(int16_growing_neighbor); Serial.print(" 0x");Serial.println(int16_growing_neighbor,HEX);
  Serial.print("int16_fading_neighbor= ");Serial.print(int16_fading_neighbor); Serial.print(" 0x");Serial.println(int16_fading_neighbor,HEX);
  
  Serial.print("output_product =");Serial.println(output_product);


  Serial.println("----------------------" );
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
  }

}
