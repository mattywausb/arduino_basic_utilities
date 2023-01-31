#include "Lamphsv.h"
#include "Arduino.h"


uint16_t Lamphsv::s_calibration_angle_60_d10[3] = {0x0400,0x0c00,0x1400}; // Angles of yellow, cyan, magenta in 60_d10
bool  Lamphsv::s_use_calibration=true;

Lamphsv::Lamphsv(void)
{
  // Initialize to white with half brightness
  m_hue_60_d10=0;
  m_saturation_d7=0;
  m_value_d7=LAMPHSV_VALUE_INTERNAL_MAX>>1;
}


static void Lamphsv::setCalibrationHueAngle (uint8_t angle_index, uint16_t angle)
{
  if(angle_index < 3) {
    while(angle<0) angle+=360;
    while(angle>359) angle-=360;
    s_calibration_angle_60_d10[angle_index]=((angle*17)+((angle/15))); 
    #ifdef LAMPHSV_TRACE_SET_CALIBRATION
      Serial.print(F("LAMPHSV_TRACE_SET_CALIBRATION > angle_index= "));Serial.print(angle_index);
      Serial.print(F(" angel in 60_d10="));Serial.println(s_calibration_angle_60_d10[angle_index]);
    #endif
  }
}

void Lamphsv::set_hsv(int16_t hue,int8_t saturation,int8_t value) 
{
  set_hue(hue);
  set_saturation(saturation);
  set_value(value);
};

// ============= HUE Methods ====================

int16_t Lamphsv::get_hue()
{
  uint32_t pure_hue=m_hue_60_d10 & ~LAMPHSV_CHANGE_BIT;
  return (pure_hue/17)-(pure_hue>LAMPHSV_HUE_DOWNSCALE_CORRECTION_BORDER?1:0);
};

void Lamphsv::set_hue(int16_t angle) 
{
  while(angle<0) angle+=360;
  while(angle>359) angle-=360;
  int16_t hue_scaled =((angle*17)+((angle/15))); 

  if(m_hue_60_d10!=hue_scaled) {
    m_hue_60_d10=hue_scaled;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }
};

void Lamphsv::add_hue_angle(int16_t delta_angle) 
{
  // Limit and scale input
  while(delta_angle<-359) delta_angle+=360;
  while(delta_angle>359) delta_angle-=360;

  int16_t scaled_delta_angle=((delta_angle*17)+((delta_angle/15))); 

  int16_t new_angle=m_hue_60_d10+scaled_delta_angle;

  while(new_angle<0) new_angle+=LAMPHSV_HUE_SCALE;
  while(new_angle>=LAMPHSV_HUE_SCALE) new_angle-=LAMPHSV_HUE_SCALE;

  if(m_hue_60_d10!=new_angle) {
    m_hue_60_d10!=new_angle;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }
}


// ============= SAATURATION Methods ====================


  /*! 
    Get saturation value 
    @return saturation value 100=full color 0= no color/white
  */
int8_t Lamphsv::get_saturation() 
{
  int16_t  downscale_value=((int16_t)m_saturation_d7*100)>>LAMPHSV_8BIT_DECIMALS;
  return downscale_value;
}  ;

/*! 
  Set saturation to  a specific value. 100=full color 0= no color/white.  Values out of bound will be clipped to lower or upper bound accordingly
  @param saturation 
*/
void Lamphsv::set_saturation(int8_t saturation)  
{
  // limit input
  if(saturation<0) saturation=0;
  if(saturation>100) saturation=100;

  //Scale input
  int16_t saturation_scaled=((int16_t)(saturation<<LAMPHSV_8BIT_DECIMALS))/100;

  if(m_saturation_d7!=saturation_scaled) {
    m_saturation_d7=saturation_scaled;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }
};

/*! 
Change saturation by an absolute increment. Resulting value will be clipped to lower or upper bound accordingly
@param increment 
*/
void Lamphsv::add_saturation(int8_t increment)
{
  // limit input
  if(increment<-100) increment=-100;
  if(increment>100) increment=100;

  // Calculate and limit nrew value

  int16_t result=(int16_t)m_saturation_d7 + ((int16_t)increment<<LAMPHSV_8BIT_DECIMALS)/100;  

  if(result>LAMPHSV_SATURATION_INTERNAL_MAX) result=LAMPHSV_SATURATION_INTERNAL_MAX;
  if(result<0) result=0;

  // commit value to member
  if(m_saturation_d7!=result) {
    m_saturation_d7=result;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }

}

void Lamphsv::multiply_saturation(int16_t factor)
{
  // limit input
  if(factor<0) factor=0;
  int16_t factor_scaled=(factor<<LAMPHSV_8BIT_DECIMALS)/100;

  // Calculate and limit nrew value

  int16_t result=((int16_t)m_saturation_d7 * factor_scaled)>>LAMPHSV_8BIT_DECIMALS;  //Shift by additional decimals

  if(result>LAMPHSV_SATURATION_INTERNAL_MAX) result=LAMPHSV_SATURATION_INTERNAL_MAX;
  if(result<0) result=0;

  // commit value to member
  if(m_saturation_d7!=result) {
    m_saturation_d7=result;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  };
}


// ============= VALUE Methods ====================


  /*! 
    Get value value 
    @return value value 100=full color 0= no color/white
  */
int8_t Lamphsv::get_value() 
{
  int16_t  downscale_value=((int16_t)m_value_d7*100)>>LAMPHSV_8BIT_DECIMALS;
  return downscale_value;
}  ;

/*! 
  Set value to  a specific value. 100=full color 0= no color/white.  Values out of bound will be clipped to lower or upper bound accordingly
  @param value 
*/
void Lamphsv::set_value(int8_t value)  
{
  // limit input
  if(value<0) value=0;
  if(value>100) value=100;

  //Scale input
  int16_t value_scaled=((int16_t)(value<<LAMPHSV_8BIT_DECIMALS))/100;

  if(m_value_d7!=value_scaled) {
    m_value_d7=value_scaled;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }
};

/*! 
Change value by an absolute increment. Resulting value will be clipped to lower or upper bound accordingly
@param increment 
*/
void Lamphsv::add_value(int8_t increment)
{
  // limit input
  if(increment<-100) increment=-100;
  if(increment>100) increment=100;

  // Calculate and limit nrew value

  int16_t result=(int16_t)m_value_d7 + ((int16_t)increment<<LAMPHSV_8BIT_DECIMALS)/100;  

  if(result>LAMPHSV_VALUE_INTERNAL_MAX) result=LAMPHSV_VALUE_INTERNAL_MAX;
  if(result<0) result=0;

  // commit value to member
  if(m_value_d7!=result) {
    m_value_d7=result;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  }

}

void Lamphsv::multiply_value(int16_t factor)
{
  // limit input
  if(factor<0) factor=0;
  int16_t factor_scaled=(factor<<LAMPHSV_8BIT_DECIMALS)/100;

  // Calculate and limit nrew value

  int16_t result=((int16_t)m_value_d7 * factor_scaled)>>LAMPHSV_8BIT_DECIMALS;  //Shift by additional decimals

  if(result>LAMPHSV_VALUE_INTERNAL_MAX) result=LAMPHSV_VALUE_INTERNAL_MAX;
  if(result<0) result=0;

  // commit value to member
  if(m_value_d7!=result) {
    m_value_d7=result;
    // set the change flag
    m_hue_60_d10 |= LAMPHSV_CHANGE_BIT;
  };
}

// ============= THe final converter ====================






#define D7_DECIMALS 7
#define D10_DECIMALS 10
#define D10_DECIMAL_MASK 0x03ff
#define VALUE_1_D7 128
#define VALUE_1_D20 0x100000
#define VALUE_1_D10 1024
#define SCALE_SHIFT 3

int16_t Lamphsv::get_calibrated_hue() {
    int16_t hue_60_d10;
    int8_t segment=m_hue_60_d10>>D10_DECIMALS; 
    int16_t segment_base_60_d10=VALUE_1_D10*segment;
    int16_t segment_distance_60_d10=m_hue_60_d10 & D10_DECIMAL_MASK;
    int8_t color_sector=segment>>1;

    int16_t calibrated_distance_60_d10;
    int16_t final_hue_60_d10;

    if((segment&0x01) == 0) { //from prime to mixed
        calibrated_distance_60_d10=s_calibration_angle_60_d10[color_sector]-segment_base_60_d10;
        final_hue_60_d10=segment_base_60_d10+(((uint32_t)segment_distance_60_d10*calibrated_distance_60_d10)>>D10_DECIMALS);        
    } else {   // from mixed to prime
        calibrated_distance_60_d10=segment_base_60_d10+VALUE_1_D10-s_calibration_angle_60_d10[color_sector];
        final_hue_60_d10 = s_calibration_angle_60_d10[color_sector]+(((uint32_t)segment_distance_60_d10*calibrated_distance_60_d10)>>D10_DECIMALS);
    };
    #ifdef LAMPHSV_TRACE_CALIBRATION
      Serial.print(F("LAMPHSV_TRACE_CALIBRATION> calibrated hue="));
      Serial.println((final_hue_60_d10/17)-(final_hue_60_d10>LAMPHSV_HUE_DOWNSCALE_CORRECTION_BORDER?1:0));
    #endif

    return final_hue_60_d10;
}; 

t_lamp_rgb_color Lamphsv::get_color_rgb() {
  t_lamp_rgb_color result;
  get_color_rgb(&result);
  return result;
};

void Lamphsv::get_color_rgb(t_lamp_rgb_color *pTarget)
{
   // reset change Flag
  m_hue_60_d10&=~LAMPHSV_CHANGE_BIT;

  // first remove the change flag, so the hue value is clean
  int16_t final_hue_60_d10;
  if(s_use_calibration) final_hue_60_d10=get_calibrated_hue();
  else final_hue_60_d10=m_hue_60_d10;

  int8_t hue_segment=final_hue_60_d10>>D10_DECIMALS;   // "Divide hue degrees by 60" (6 Segments starting with red - yellow- green- cyan - blue - magenta)
  int16_t hue_segment_fraction_d10=final_hue_60_d10 & D10_DECIMAL_MASK; //Get last 10 Bits = "Modulo 60" = fraction inside in the segment
  int16_t white_factor_d7=((int16_t)m_value_d7*((int16_t)VALUE_1_D7-(int16_t)m_saturation_d7))>>D7_DECIMALS; // >>7 for Decimal point shifting

  int32_t partner_segment_fraction_d20;
  if(hue_segment%2==0)  partner_segment_fraction_d20 = ((int32_t)VALUE_1_D20 - (((int32_t)m_saturation_d7)<<SCALE_SHIFT) * (VALUE_1_D10-hue_segment_fraction_d10) );
  else                  partner_segment_fraction_d20 = ((int32_t)VALUE_1_D20 - (((int32_t)m_saturation_d7)<<SCALE_SHIFT) *               hue_segment_fraction_d10  ) ;

  int16_t partner_factor_d10= ((((int32_t)m_value_d7)<<SCALE_SHIFT) *partner_segment_fraction_d20)>>(2*D10_DECIMALS);

  #define scale_d7_to_255(b) b==VALUE_1_D7?255:b<<1
  #define scale_d10_to_255(b) b>=VALUE_1_D10?255:b>>2

  switch (hue_segment)
  {
     case 0:  // from red to yellow 0-59
            pTarget->r=scale_d7_to_255(m_value_d7);
            pTarget->g=scale_d10_to_255(partner_factor_d10);
            pTarget->b=scale_d7_to_255(white_factor_d7);
            break;
     case 1: // from yellow to green 60-119
            pTarget->g=scale_d7_to_255(m_value_d7);
            pTarget->r=scale_d10_to_255(partner_factor_d10);
            pTarget->b=scale_d7_to_255(white_factor_d7);
            break;
     case 2: // from green to cyan   120-179
            pTarget->g=scale_d7_to_255(m_value_d7);
            pTarget->b=scale_d10_to_255(partner_factor_d10);
            pTarget->r=scale_d7_to_255(white_factor_d7);
            break;
     case 3: // from cyan to blue   180-239
            pTarget->b=scale_d7_to_255(m_value_d7);
            pTarget->g=scale_d10_to_255(partner_factor_d10);
            pTarget->r=scale_d7_to_255(white_factor_d7);
            break;
     case 4: // from blue to magenta 240-299
            pTarget->b=scale_d7_to_255(m_value_d7);
            pTarget->r=scale_d10_to_255(partner_factor_d10);
            pTarget->g=scale_d7_to_255(white_factor_d7);
            break;
     case 5: // from magenta to red 300-359
            pTarget->r=scale_d7_to_255(m_value_d7);
            pTarget->b=scale_d10_to_255(partner_factor_d10);
            pTarget->g=scale_d7_to_255(white_factor_d7);
            break;

   } // switch

}


// ===============  DEBUG STUFF ==================

#ifdef LAMPHSV_ADD_TRACE_METHODS
void Lamphsv::print_members_to_serial()
{
    Serial.print(F("m_hue_60_d10= "));Serial.print(m_hue_60_d10);Serial.print(F(" 0x"));Serial.print(m_hue_60_d10,HEX);Serial.print(F(" p;"));Serial.println(m_hue_60_d10&~LAMPHSV_CHANGE_BIT);
    Serial.print(F("m_saturation_d7= "));Serial.print(m_saturation_d7);Serial.print(F(" 0x"));Serial.println(m_saturation_d7,HEX);
    Serial.print(F("m_value_d7= "));Serial.print(m_value_d7);Serial.print(F(" 0x"));Serial.println(m_value_d7,HEX);
};


//void print_rgb_to_serial();
#endif
