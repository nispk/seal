#include "seal/seal.h"
#include "math.h"
#include <iostream>
#include "sigmoid.h"

using namespace std;
using namespace seal;


/*
This function creates neural network based on the inputs provided.
We give network feature vector inputs in the form of an array of ciphertexts. 
Each value in the array is an encryption of packed plaintext of 105 input samples.
It returns an output ciphertext which is an encryption of output of the 105 input samples.
*/

inline void neural_network(
    shared_ptr<SEALContext> context, Ciphertext inputs[], vector<Ciphertext> weights, vector<Ciphertext> bias,
    size_t layers, size_t inp_per_layer[], size_t out_per_layer[], Ciphertext &output, int q,  RelinKeys rk)
{
    Evaluator evaluator(context);

    vector<Ciphertext>::iterator wt_ptr = weights.begin();
    vector<Ciphertext>::iterator bs_ptr = bias.begin();

    vector<vector<Ciphertext>> input_matrix(layers);
    
 
    for (size_t n = 0; n < layers; n++)
    {
        size_t col = inp_per_layer[n];
        input_matrix[n] = vector<Ciphertext>(col);
    }


    for (size_t m = 0; m < inp_per_layer[0]; m++)
    {
        input_matrix[0][m] = inputs[m];
    }

    for (size_t i = 1; i < layers + 1; i++)
    {
        size_t output_nodes = out_per_layer[i - 1];
        size_t input_nodes = inp_per_layer[i - 1];
        Ciphertext new_bias, Percep_op;

        // No. of weights_per_layer = no. of input_per_layer x no. of outputs_per_layer

        for (size_t j = 0; j < output_nodes; j++)
        {
            Ciphertext sig_result, x, y;
            vector<Ciphertext> temp(input_nodes);

            //cout << " compute perceptron output " << endl;
            for (size_t k = 0; k < input_nodes; k++)
            {
                Ciphertext enc1, enc2;
                enc1 = input_matrix[i - 1][k];
                enc2 = *wt_ptr;

                if (enc1.parms_id() != enc2.parms_id())
                    evaluator.mod_switch_to_inplace(enc2, enc1.parms_id());
                                   
                evaluator.multiply(enc1, enc2, temp[k]);

                evaluator.relinearize_inplace(temp[k], rk);
                evaluator.rescale_to_next_inplace(temp[k]);
                wt_ptr++;
                enc1.release();
                enc2.release();
            }

            
            for (size_t l = 1; l < input_nodes; l++)
                evaluator.add_inplace(temp[0], temp[l]);

            x = temp[0];
            new_bias = *bs_ptr;
            new_bias.scale() = x.scale();
            evaluator.mod_switch_to_inplace(new_bias, x.parms_id());
            evaluator.add_inplace(x, new_bias);

            if (i != 1)
                evaluator.rescale_to_next_inplace(x);

            sigmoid(context, x, Percep_op, q , rk);

            if (i != layers)
                input_matrix[i][j] = Percep_op;   
            else
                output = Percep_op;

            bs_ptr++;
            temp.clear();
            sig_result.release();
            x.release();
            y.release();
            Percep_op.release();
            new_bias.release();
        }
    }
   

}
