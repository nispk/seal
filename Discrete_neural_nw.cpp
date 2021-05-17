#include "performance_test.h"
#include <fstream>
#include <iostream>
#include "sign_function.h"

using namespace std;
using namespace seal;


/*
* This method performs homomorphic validation of discrete neural network with pre-trained weights and biases.
* The neural network comrises single neuron that performs linear classification on 10 input samples
using sign activation function.
*/

void discrete(shared_ptr<SEALContext> context)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);
    KeyGenerator keygen(context);
    PublicKey public_key = keygen.public_key();
    SecretKey secret_key = keygen.secret_key();
    RelinKeys relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    BatchEncoder encoder(context);

    int64_t input1, input2, disc_weight1, disc_weight2, disc_bias;
    Plaintext input_plain1, input_plain2, weight_plain1, weight_plain2, bias_plain, max_input_plain;
    Ciphertext enc_input1, enc_input2, enc_weight1, enc_weight2, enc_bias, sign_output;

    vector<int64_t> vec_in1;
    vector<int64_t> vec_in2;
    vector<int64_t> vec_3;
    vector<Ciphertext> func_enc;
    vector<Plaintext> func_indices;

    int64_t n = 6;
    int64_t N = pow(2, n);
    int64_t max_input_range = 32;
    int samples = 10;

    // Compute ciphertexts required for neural network computations
    double input_sample[samples][2] = { { 2.7810836, 2.550537003 },    { 1.465489372, 2.362125076 },
                                        { 3.396561688, 4.400293529 },  { 1.38807019, 1.850220317 },
                                        { 3.06407232, 3.005305973 },   { 7.627531214, 2.759262235 },
                                        { 5.332441248, 2.088626775 },  { 6.922596716, 1.77106367 },
                                        { 8.675418651, -0.242068655 }, { 7.673756466, 3.508563011 } };

    double weights[2] = { 3.980833, -6.857337 };
    double bias = -3;
    int64_t label[samples] = { 0, 0, 0, 0, 0, 1, 1, 1, 1, 1 };

    disc_weight1 = discreteW(weights[0]);
    disc_weight2 = discreteW(weights[1]);
    disc_bias = static_cast<int64_t>(bias);
    disc_bias = discreteW(bias);
    vector<int64_t> vec_wght1(samples, disc_weight1);
    vector<int64_t> vec_wght2(samples, disc_weight2);
    vector<int64_t> vec_bias(samples, disc_bias);

    for (int i = 0; i < samples; i++)
    {
        input1 = discreteW(input_sample[i][0]);
        input2 = discreteW(input_sample[i][1]);
        vec_in1.push_back(input1);
        vec_in2.push_back(input2);
    }

    // Compute the function lookup table
    Plaintext zplain;
    Ciphertext z_enc;
    vector<int64_t> z(samples);
    int64_t sign_value;

    time_start = chrono::high_resolution_clock::now();

    for (int64_t l = 1; l <= N; l++)
    {
        sign_value = sign(l - max_input_range);
        fill(z.begin(), z.end(), sign_value);
        encoder.encode(z, zplain);
        encryptor.encrypt(zplain, z_enc);
        func_enc.push_back(z_enc);
    }

    sign_value = sign(-max_input_range);
    fill(z.begin(), z.end(), sign_value);
    encoder.encode(z, zplain);
    encryptor.encrypt(zplain, z_enc);
    func_enc.push_back(z_enc);

    // Compute the index vector plaintext of lookup table
    for (int64_t j = 0; j <= N; j++)
    {
        vector<int64_t> vec(samples, j);
        Plaintext ptxt;
        encoder.encode(vec, ptxt);
        func_indices.push_back(ptxt);
    }

    // Compute the preamble vector of the lookup table
    vector<int64_t> vec = { -168516, -196777, -389729, 241728,  244996,  212316,  150719,  -385744, -133877, 343107,
                            317486,  39564,   -158256, -209929, -348814, 380114,  -101787, 134705,  46393,   92224,
                            -343028, -258957, -313555, 175208,  191527,  105926,  54809,   -381382, -150283, 44258,
                            78303,   -105326, 78303,   44258,   -150283, -381382, 54809,   105926,  191527,  175208,
                            -313555, -258957, -343028, 92224,   46393,   134705,  -101787, 380114,  -348814, -209929,
                            -158256, 39564,   317486,  343107,  -133877, -385744, 150719,  212316,  244996,  241728,
                            -389729, 196777,  65969,   -46519,  -46519 };

    vector<Plaintext> preamble;
    for (int k = 0; k <= N; k++)
    {
        vector<int64_t> sample_vec(samples, vec[k]);
        encoder.encode(sample_vec, zplain);
        preamble.push_back(zplain);
    }

    // Compute the max_input_range plaintext required to compute sign function input " << endl;
    vector<int64_t> mir(samples, max_input_range);
    encoder.encode(mir, max_input_plain);

    encoder.encode(vec_wght1, weight_plain1);
    encoder.encode(vec_wght2, weight_plain2);
    encoder.encode(vec_bias, bias_plain);
    encoder.encode(vec_in1, input_plain1);
    encoder.encode(vec_in2, input_plain2);
    encryptor.encrypt(weight_plain1, enc_weight1);
    encryptor.encrypt(weight_plain2, enc_weight2);
    encryptor.encrypt(bias_plain, enc_bias);
    encryptor.encrypt(input_plain1, enc_input1);
    encryptor.encrypt(input_plain2, enc_input2);

    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    size_t input_size = (enc_input1.int_array().size()) / 1024 + (enc_input2.int_array().size()) / 1024 +
                        (enc_weight1.int_array().size()) / 1024 + (enc_weight2.int_array().size()) / 1024 +
                        (enc_bias.int_array().size()) / 1024;

    cout << " + Total size of all the input ciphertexts to the network is : " << input_size << " KB . " << endl;
    cout << " + Time taken for encoding and encrypting network inputs is : " << time_diff.count() << " microseconds . "
         << endl;

    // Neuron computation begins

    time_start = chrono::high_resolution_clock::now();
    evaluator.multiply_inplace(enc_input1, enc_weight1);
    evaluator.relinearize_inplace(enc_input1, relin_keys);
    evaluator.multiply_inplace(enc_input2, enc_weight2);
    evaluator.relinearize_inplace(enc_input2, relin_keys);
    evaluator.relinearize_inplace(enc_input2, relin_keys);
    evaluator.add_inplace(enc_input1, enc_input2);
    evaluator.add_inplace(enc_input1, enc_bias);

    // Compute the input to the sign function

    evaluator.add_plain_inplace(enc_input1, max_input_plain);
    // evaluator.mod_switch_to_inplace(enc_input1, context->last_parms_id());

    algo2_sign(context, enc_input1, func_enc, func_indices, preamble, sign_output, N, relin_keys);

    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    cout << " + Time taken to compute discrete neuron output is : " << time_diff.count() << " microseconds. " << endl;

    vector<int64_t> val1;
    Plaintext plain1;
    evaluator.mod_switch_to_inplace(sign_output, context->last_parms_id());
    decryptor.decrypt(sign_output, plain1);

    cout << " + Storage space of ciphertext returned by activation function is : "
         << (sign_output.int_array().size()) / 1024 << " KB ." << endl;

    encoder.decode(plain1, val1);

    cout << " + Noise budget in the output ciphertext is :" << decryptor.invariant_noise_budget(sign_output)
         << " bits . " << endl;

    int64_t y[samples];
    int count = 0;
    for (int i = 0; i < samples; i++)
    {
        y[i] = val1.at(i);
        if (y[i] == -1)
            y[i] = 0;

        if (label[i] == y[i])
            count++;
    }

    cout << " + Total number of input samples for linear classification are: " << samples << endl;
    cout << " + Total number of correctly classified samples are : " << count << endl;
    cout << " + Validation accuracy is " << static_cast<double>(count / samples) * 100 << " percent ." << endl;
    cout << endl;
}