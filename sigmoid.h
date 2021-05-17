#include "seal/seal.h"
#include "math.h"
#include <iostream>

using namespace std;
using namespace seal;

/*
* This function computes approximate sigmoid(x) where x is in [-8,8]
* The approximation formula is: 
*  g7(x) = b0 + b1 * (x/8) + b3 * (x/8)^3 + b5 * (x/8)^5 + b7 * (x/8)^7
*/

inline void sigmoid(shared_ptr<SEALContext> &context, Ciphertext &input, Ciphertext &output1, int q, RelinKeys rk)
{
    double scale = pow(2.0, q);
    // print_parameters(context);
    cout << endl;
    Evaluator evaluator(context);
    CKKSEncoder encoder(context);
    size_t slot_count = encoder.slot_count();

    double b0 = 0.5, b1 = 1.73496, b3 = 4.19407, b5 = 5.43402, b7 = 2.50739;

    Plaintext one_upon_eight, plain_x, x_by_eight, plain_b0, plain_b1, plain_b3, plain_b5, plain_b7, plain_g7;
    Ciphertext enc_x_by_eight, enc_x_by_eight_square, enc_x_by_eight_cube, enc_x_by_eight_5, enc_x_by_eight_7;
    Ciphertext array_b[5];

    //Compute b1*(x/8) 
    encoder.encode(b1, scale, plain_b1);
    encoder.encode(0.125, scale, one_upon_eight);
    evaluator.mod_switch_to_inplace(one_upon_eight, input.parms_id());
    evaluator.multiply_plain(input, one_upon_eight, enc_x_by_eight);
    evaluator.rescale_to_next_inplace(enc_x_by_eight);
    evaluator.mod_switch_to_inplace(plain_b1, enc_x_by_eight.parms_id());
    evaluator.multiply_plain(enc_x_by_eight, plain_b1, array_b[1]);


    // compute cube of x/8
    evaluator.square(enc_x_by_eight, enc_x_by_eight_square);
    // cout << " x/8^2 scale is " << log2(enc_x_by_eight_square.scale()) << endl;
    evaluator.relinearize_inplace(enc_x_by_eight_square, rk);
    evaluator.multiply(enc_x_by_eight_square, enc_x_by_eight, enc_x_by_eight_cube);
    evaluator.rescale_to_next_inplace(enc_x_by_eight_cube);
    evaluator.relinearize_inplace(enc_x_by_eight_cube, rk);
    // cout << " (x/8)^3 scale is " << log2(enc_x_by_eight_cube.scale()) << endl;

    // Compute b3*(x/8)^3 
    encoder.encode(b3, scale, plain_b3);
    evaluator.mod_switch_to_inplace(plain_b3, enc_x_by_eight_cube.parms_id());
    evaluator.multiply_plain(enc_x_by_eight_cube, plain_b3, array_b[2]);
    evaluator.rescale_to_next_inplace(array_b[2]);
   

    // Compute b5*(x/8)^5 " << endl;
    evaluator.rescale_to_next_inplace(enc_x_by_eight_square);
    evaluator.square(enc_x_by_eight_square, enc_x_by_eight_5);
    evaluator.relinearize_inplace(enc_x_by_eight_5, rk);
    evaluator.rescale_to_next_inplace(enc_x_by_eight_5);
    evaluator.mod_switch_to_inplace(enc_x_by_eight, enc_x_by_eight_5.parms_id());
    evaluator.multiply_inplace(enc_x_by_eight_5, enc_x_by_eight);
    evaluator.relinearize_inplace(enc_x_by_eight_5, rk);
    evaluator.rescale_to_next_inplace(enc_x_by_eight_5);


    // update **
    encoder.encode(b5, scale, plain_b5);
    evaluator.mod_switch_to_inplace(plain_b5, enc_x_by_eight_5.parms_id());
    evaluator.multiply_plain(enc_x_by_eight_5, plain_b5, array_b[3]);


    // compute b7 *(x/8)^7 and add it to enc_g7
    evaluator.mod_switch_to_inplace(enc_x_by_eight_square, enc_x_by_eight_5.parms_id());
    evaluator.multiply(enc_x_by_eight_5, enc_x_by_eight_square, enc_x_by_eight_7);
    evaluator.relinearize_inplace(enc_x_by_eight_7, rk);
    evaluator.rescale_to_next_inplace(enc_x_by_eight_7);

    encoder.encode(b7, scale, plain_b7);
    evaluator.mod_switch_to_inplace(plain_b7, enc_x_by_eight_7.parms_id());
    evaluator.multiply_plain(enc_x_by_eight_7, plain_b7, array_b[4]);


    // check the chain index and scale of all the values to be added
    //if ((log2(array_b[4].scale()) <= 60))
    if ((log2(array_b[4].scale()) <= 2 * q))
    {
        for (int i = 1; i < 5; i++)
        {
            evaluator.mod_switch_to_inplace(array_b[i], array_b[4].parms_id());
            array_b[i].scale() = pow(2, 2*q);
        }
    }
    else
    {
        for (int i = 1; i < 5; i++)
        {
            evaluator.mod_switch_to_inplace(array_b[i], array_b[4].parms_id());
            array_b[i].scale() = array_b[4].scale();

        }
    }

    encoder.encode(b0, array_b[1].scale(), plain_b0);
    evaluator.mod_switch_to_inplace(plain_b0, array_b[1].parms_id());

    // compute g7(x)

    evaluator.add_plain(array_b[1], plain_b0, array_b[0]);
    evaluator.sub_inplace(array_b[0], array_b[2]);
    evaluator.add_inplace(array_b[0], array_b[3]);
    evaluator.sub(array_b[0], array_b[4], output1);

}
