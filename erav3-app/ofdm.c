#include <complex.h>
//#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>

#include "debug.h"
#include "type.h"
#include "base.h"
#include "ofdm.h"
#include "viterbi_flat.h"

// typedef ap_fixed<32,15> fx_pt1_ext1;
// typedef ap_fixed<64,16> fx_pt1_ext2;
// typedef complex< fx_pt1_ext1 > fx_pt_ext1;
// typedef complex< fx_pt1_ext2 > fx_pt_ext2;


void decode_signal( unsigned num_inputs, fx_pt constellation[DECODE_IN_SIZE_MAX], unsigned* num_outputs, uint8_t * output_data ) // hls::stream< ap_uint<1> > &output_data  )
{
  // JDW : REPLACING ALL of this with our viterbi (library) version...
  // ap_uint<1> bit_r[CHUNK];
  // ap_uint<1> bit[CHUNK];
  unsigned num_sym = num_inputs/48;
  uint8_t bit_r[DECODE_IN_SIZE_MAX];
  uint8_t bit[DECODE_IN_SIZE_MAX];

  DEBUG(printf("In the decode_signal routine...\n"));
  // map to the nearest one
  for(unsigned i = 0; i < num_inputs /*DECODE_IN_SIZE_MAX*/;i++) {
    if( crealf(constellation[i]) > 0 ) {
      bit_r[i] = 1;
    } else {
      bit_r[i] = 0;
    }
    DEBUG(printf(" OFDM_BIT_R %5u : CONS %12.8f %12.8f : BIT_R %u\n", i, crealf(constellation[i]), cimagf(constellation[i]), bit_r[i]));
  }

  DEBUG(printf("     at the interleaving...\n"));
  // interleaving
  const unsigned inter[]=
    { 0,3,6,9,12,15,18,21,24,27,30,33,36,
      39,42,45,1,4,7,10,13,16,19,22,25,28,
      31,34,37,40,43,46,2,5,8,11,14,17,20,23,
      26,29,32,35,38,41,44,47 };

  for (unsigned i = 0; i < num_sym; i++) {
    for (unsigned j = 0; j < 48; j++) {
      unsigned index = inter[j]+i*48;
      bit[ j+i*48 ] = bit_r[index];
      DEBUG(printf(" OFDM_BIT %5u : BIT %5u = BIR_R %5u = %u\n", i, (j+i*48), index, bit[j+i*48]));
    }
  }

  DEBUG(printf("     at the call to our decode...\n"));
  unsigned num_out_bits = num_inputs/2;
  {
    ofdm_param ofdm = {   BPSK_1_2, //  encoding   : 0 = BPSK_1_2
			  13,       //             : rate field of SIGNAL header //Taken constant
			  1,        //  n_bpsc     : coded bits per subcarrier
			  48,       //  n_cbps     : coded bits per OFDM symbol
			  24 };     //  n_dbps     : data bits per OFDM symbol

    frame_param frame = {  1528,     // psdu_size      : PSDU size in bytes
			   (int)(num_sym),      // n_sym          : number of OFDM symbols
			   18,       // n_pad          : number of padding bits in DATA field
			   (int)num_inputs, // 24528,    // n_encoded_bits : number of encoded bits
			   (int)num_out_bits } ; //12264 };  // n_data_bits: number of data bits, including service and padding
    int n_res_char;
    DEBUG(printf("Calling decode with OFDM_PARMS %u %2u %u %2u %2u\n", ofdm.encoding, 13, ofdm.n_bpsc, ofdm.n_cbps, ofdm.n_dbps);
	  printf("               and FRAME_PARMS %4u %3u %2u %5u %5u\n",  frame.psdu_size, frame.n_sym, frame.n_pad, frame.n_encoded_bits, frame.n_data_bits));
    DEBUG(printf("Calling decode : n_inputs = %u \n", num_inputs));
    decode(&ofdm, &frame, bit /*input_data*/, &n_res_char, output_data);
    // end of decode (viterbi) function, but result bits need to be "descrambled"
    *num_outputs = num_out_bits;
  }
  DEBUG(printf("  done and leaving ofdm.c\n"));
}


