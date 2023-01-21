#define TRACE_ON
//#define TRACE_SAMPLES_HIGH

byte pin_list[]={2,4};//,6,7};
#define PIN_COUNT sizeof(pin_list)
byte pin_mask=0;
byte prev_pattern=0;


struct sample_buffer_struct {
  uint16_t timestamp;
  byte pattern;
};

// Max idle time is defined in microseconds >> 8
#define MAX_IDLE_TIME 1000

#define SAMPLE_COUNT 550
struct sample_buffer_struct sample[SAMPLE_COUNT];
int sample_index=0;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); // light LED during setup

  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif

  for(byte p=0;p<PIN_COUNT;p++) {
    if(p<2)pinMode(pin_list[p],INPUT);
    else pinMode(pin_list[p],INPUT_PULLUP);
    bitWrite(pin_mask,pin_list[p],1);
  }
  Serial.print("pin_mask=");Serial.println(0x0100|pin_mask,BIN);
  digitalWrite(LED_BUILTIN, LOW); 
}

void loop() {
  byte current_pattern=(PIND&pin_mask);
  uint16_t current_time=(millis()); // we only care about the last 650000 milliseconds with better resolution (about 1 Minute)

  #ifdef TRACE_SAMPLES_HIGH
        Serial.print(current_time); Serial.print(":");
        Serial.println(0x0100|current_pattern,BIN) ;
  #endif

  // collect samples as fast as possible
  if(current_pattern^prev_pattern || sample_index==0) {
      sample[sample_index].timestamp=current_time;
      sample[sample_index].pattern= current_pattern;
      sample_index++;
      prev_pattern=current_pattern;
      digitalWrite(LED_BUILTIN, sample_index&0x0003); 
      #ifdef TRACE_SAMPLES_HIGH
        Serial.print("#");Serial.println(sample_index); 
      #endif
  }

  // Dump data from sample buffer to serial
  if((sample_index>1 && current_time-sample[sample_index-1].timestamp>MAX_IDLE_TIME)||sample_index>=SAMPLE_COUNT) {
    uint16_t sample_start_time=sample[0].timestamp;
    uint8_t pattern_difference=0;
    prev_pattern=sample[0].pattern;
    digitalWrite(LED_BUILTIN, LOW); 
    Serial.println(F("------ DATA --------"));
    for(int i=0;i<sample_index;i++) {
        pattern_difference=sample[i].pattern^prev_pattern;
        prev_pattern=sample[i].pattern;
        Serial.print((sample[i].timestamp-sample_start_time)); Serial.print(":\t");
        for(uint8_t b=7;b<=7;b--) {
          if(bitRead(pin_mask,b)) {Serial.print(bitRead(sample[i].pattern,b));
            if(bitRead(pattern_difference,b) )Serial.print("<");
               else  Serial.print(" ");
          }
          Serial.print("  ");
        }
        Serial.println() ;
    }
    Serial.println(F("------ END --------"));
    delay(500);
    Serial.println(F("------ MEASURE --------"));
    sample_index=0;
  }
  

}
