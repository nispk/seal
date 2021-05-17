#include <fstream>
#include <iostream>
#include "examples.h"
#include "neural_nw.h"
#include "sign_function.h"

using namespace std;
using namespace seal;


int64_t sign(int64_t x)
{
    int64_t z = 1;
    int64_t z_ = -1;
    if (x < 0)
    {
        return z_;
    }
    else if (x > 0)
    {
        return z;
    }
    else
        return 0;
}

int64_t discrete(double x)
{
    // int scale = 10;
    // return static_cast<int>(scale * lround(x / scale));
    return static_cast<int64_t>(lround(x));
}

/*
This function is used to discretize the weights and biases of the network
*/
int64_t discreteW(double x)
{
    int scale = 2;
    return static_cast<int64_t>(scale * lround(x / scale));
}

void sign1(shared_ptr<SEALContext> context, int p)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);

    auto &parms = context->first_context_data()->parms();
    auto &plain_modulus = parms.plain_modulus();
    auto &coeff_modulus = parms.coeff_modulus();
    size_t poly_modulus_degree = parms.poly_modulus_degree();
    print_parameters(context);
    cout << endl;

    KeyGenerator keygen(context);
    auto secret_key = keygen.secret_key();
    auto public_key = keygen.public_key();
    auto relin_keys = keygen.relin_keys_local();

    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);
    Evaluator evaluator(context);
    BatchEncoder encoder(context); // Only testing batch encoder implementation

    vector<Plaintext> func_indices;
    vector<Ciphertext> func_enc;
    vector<int64_t> z;
    vector<int64_t> num_vec;
    vector<int64_t> den_vec;
    Plaintext v_plain, max_plain, zplain, in_plain;
    Ciphertext in_enc, z_enc, numerator, denominator;

    int64_t max_input_range, sign_value;

    cout << " Enter the maximum input value " << endl;
    cout << endl;
    cin >> max_input_range;
    cout << " [ " << -max_input_range << " , " << max_input_range << " ] " << endl;
    int64_t N = 2 * max_input_range;
    int64_t n = static_cast<int64_t>(log2(N));

    for (int64_t l = 1; l <= N; l++)
    {
        sign_value = sign(l - max_input_range);
        z.push_back(sign_value);
        encoder.encode(z, zplain);
        encryptor.encrypt(zplain, z_enc);
        func_enc.push_back(z_enc);
        z.clear();
    }

    sign_value = sign(-max_input_range);
    z.push_back(sign_value);
    encoder.encode(z, zplain);
    encryptor.encrypt(zplain, z_enc);
    func_enc.push_back(z_enc);
    z.clear();

    for (int64_t j = 0; j <= N; j++)
    {
        vector<int64_t> vec = { j };
        Plaintext ptxt;
        encoder.encode(vec, ptxt);
        func_indices.push_back(ptxt);
        vec.clear();
    }

    cout << " The input function lookup table size is " << endl;
    size_t lookup_table_size = (func_enc[0].int_array().size()) * func_enc.size() / 1024;
    cout << lookup_table_size << " KB . " << endl;

    vector<int64_t> mir = { max_input_range };
    encoder.encode(mir, max_plain);
    int count = 0;

    cout << " Running tests.. " << endl;

    for (int i = 0; i < 10; i++)
    {
        int64_t input = i - max_input_range;
        if (input <= -max_input_range)
            input += max_input_range;
        else if (input >= max_input_range)
            input -= max_input_range;

        vector<int64_t> v = { input };
        encoder.encode(v, v_plain);
        encryptor.encrypt(v_plain, in_enc);
        evaluator.add_plain_inplace(in_enc, max_plain);

        time_start = chrono::high_resolution_clock::now();
        algo1_sign(context, in_enc, func_enc, func_indices, numerator, denominator, N, relin_keys);
        time_end = chrono::high_resolution_clock::now();
        time_diff += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        Plaintext num_plain, den_plain;
        
        evaluator.mod_switch_to_inplace(numerator, context->last_parms_id());
        evaluator.mod_switch_to_inplace(denominator, context->last_parms_id());
        decryptor.decrypt(numerator, num_plain);
        decryptor.decrypt(denominator, den_plain);
        encoder.decode(num_plain, num_vec);
        encoder.decode(den_plain, den_vec);
        count++;
            
    }
    cout << " + Noise budget in numerator ciphertext is : " << decryptor.invariant_noise_budget(numerator) << " bits . "
         << endl;
    cout << " + Noise budget in denominator ciphertext is : " << decryptor.invariant_noise_budget(denominator) << " bits . "
         << endl;
    cout << " + Numerator storage space is : " << (numerator.int_array().size()) / 1024 << " KB . " << endl;
    
    cout << " + Denominator storage space is : " << (denominator.int_array().size()) / 1024 << " KB ." << endl;

    cout << " + Average computation time for algorithm 1 is : " << (time_diff.count()) / count
         << " microseconds " << endl;
    cout << endl;

}


void sign2(shared_ptr<SEALContext> context)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);

    auto &parms = context->first_context_data()->parms();
    auto &plain_modulus = parms.plain_modulus();
    auto &coeff_modulus = parms.coeff_modulus();
    size_t poly_modulus_degree = parms.poly_modulus_degree();
    print_parameters(context);
    cout << endl;

    KeyGenerator keygen(context);
    auto secret_key = keygen.secret_key();
    auto public_key = keygen.public_key();
    auto relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Decryptor decryptor(context, secret_key);
    Evaluator evaluator(context);
    BatchEncoder encoder(context);

    vector<Plaintext> func_indices;
    vector<Ciphertext> func_enc;
    vector<int64_t> z;
    vector<int64_t> result_vec;
    vector<int64_t> inputs;
    vector<int64_t> outputs;
    Plaintext v_plain, max_plain, zplain, in_plain, out_plain;
    Ciphertext in_enc, z_enc, output;

    int64_t max_input_range = 32;
    cout << " The input range is " << endl;
    //cin >> max_input_range;
    int64_t sign_value;
    cout << " [ " << -max_input_range << " , " << max_input_range << " ] " << endl;
    cout << endl;
    int64_t N = 2 * max_input_range;
    int64_t n = static_cast<int64_t>(log2(N));

    for (int64_t l = 1; l <= N; l++)
    {
        sign_value = sign(l - max_input_range);
        z.push_back(sign_value);
        encoder.encode(z, zplain);
        encryptor.encrypt(zplain, z_enc);
        func_enc.push_back(z_enc);
        z.clear();
    }

    sign_value = sign(-max_input_range);
    z.push_back(sign_value);
    encoder.encode(z, zplain);
    encryptor.encrypt(zplain, z_enc);
    func_enc.push_back(z_enc);
    z.clear();

    for (int64_t j = 0; j <= N; j++)
    {
        vector<int64_t> vec = { j };
        Plaintext ptxt;
        encoder.encode(vec, ptxt);
        func_indices.push_back(ptxt);
        vec.clear();
    }

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
        vector<int64_t> sample_vec = { vec[k] };
        encoder.encode(sample_vec, zplain);
        // encryptor.encrypt(zplain, enc);
        preamble.push_back(zplain);
    }

    cout << " + Sign function lookup table size is : " << (func_enc[0].int_array().size()) * func_enc.size() / 1024 << " KB . " << endl;
    cout << " + Sign function preamble vector size is : " << (preamble[0].int_array().size()) * preamble.size() / 1024 << " KB . " << endl;

    vector<int64_t> mir = { max_input_range };
    encoder.encode(mir, max_plain);
    int count = 0;
    for (int i = 0; i < 10; i++)
    {
        vector<int64_t> v = { i };
        encoder.encode(v, v_plain);
        encryptor.encrypt(v_plain, in_enc);
        evaluator.add_plain_inplace(in_enc, max_plain);

        time_start = chrono::high_resolution_clock::now();
        algo2_sign(context, in_enc, func_enc, func_indices, preamble, output, N, relin_keys);
        time_end = chrono::high_resolution_clock::now();

        cout << endl;
        evaluator.mod_switch_to_inplace(output, context->last_parms_id());
        decryptor.decrypt(output, out_plain);
        encoder.decode(out_plain, result_vec);
        count++;

        inputs.push_back(i - max_input_range);
        outputs.push_back(result_vec[0]);
    }
    cout << " Inputs to the sign function are " << endl;
    print_vector(inputs, 12);
    cout << " Outputs of the sign function are " << endl;
    print_vector(outputs, 12);
    cout << endl;

    cout << " + Output storage space is : " << (output.int_array().size()) / 1024 << " KB . " << endl;
    cout << " + Noise budget in output ciphertext is : " << decryptor.invariant_noise_budget(output) << " bits . "
         << endl;
    cout << " + Average computation time of encrypted sign function for algorithm 2 is : " << time_diff.count() / count
         << " microseconds ." << endl;

}

void sigmoid_setup(shared_ptr<SEALContext> context, int q)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);
    print_parameters(context);
    cout << endl;
    KeyGenerator keygen(context);
    auto public_key = keygen.public_key();
    auto secret_key = keygen.secret_key();
    auto relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);
    size_t slot_count = encoder.slot_count();
    double scale = pow(2.0, q);

    int count = 0;
    Plaintext plain_x, out_plain;
    Ciphertext enc_x, output;
    vector<double> result;
    double error[17];
    for (int i = 0; i < 17; i++)
    {
        double range = 8;
        double x = i - range;
        encoder.encode(x, scale, plain_x);
        encryptor.encrypt(plain_x, enc_x);
        time_start = chrono::high_resolution_clock::now();
        sigmoid(context, enc_x, output, q, relin_keys);
        time_end = chrono::high_resolution_clock::now();
        time_diff += chrono::duration_cast<chrono::microseconds>(time_end - time_start);

        decryptor.decrypt(output, out_plain);
        encoder.decode(out_plain, result);
        cout << endl;

        // HE error computation
        double b0 = 0.5, b1 = 1.96798, b3 = 5.43594, b5 = 7.48602, b7 = 3.56812;

        double original[5];
        double frac_x = x / 10;
        original[0] = b0;
        original[1] = b1 * frac_x;
        double cube = pow(frac_x, 3);
        original[2] = b3 * cube;
        double fifth = pow(frac_x, 5);
        original[3] = b5 * fifth;
        double seventh = pow(frac_x, 7);
        original[4] = b7 * seventh;
        original[5] = original[0] + original[1] - original[2] + original[3] - original[4];

        error[i] = result[0] - original[5];
        result.clear();
        count++;
    }

    cout << " + Size of sigmoid function output ciphertext is : " << (output.int_array().size()) / 1024 << " KB "
         << endl;
    cout << " + Average computation time for sigmoid calculation is : " << time_diff.count() / count
         << " microseconds. " << endl;
    cout << " + Error in Homomorphic evaluation of sigmoid function is : " << endl;
    for (int j = 1; j < 17; j++)
        error[0] += error[j];
    double mean_error = error[0] / 17;
    cout << mean_error;
    cout << endl;
    cout.flush();
}

void dnn(shared_ptr<SEALContext> context, int q)
{
    chrono::high_resolution_clock::time_point time_start, time_end;
    chrono::microseconds time_diff(0);
    print_parameters(context);
    cout << endl;
    KeyGenerator keygen(context);
    auto public_key = keygen.public_key();
    auto secret_key = keygen.secret_key();
    auto relin_keys = keygen.relin_keys_local();
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);
    CKKSEncoder encoder(context);
    size_t slot_count = encoder.slot_count();

    vector<double> x1;
    vector<double> x2;
    vector<int> y;
    vector<int> y_pred;

    ifstream myFile("dnn.csv");
    int count = 0;
    vector<string> row;
    string line, word, temp;
    double csv_x1, csv_x2, csv_a1, csv_a2, csv_a3;
    int csv_y;

    if (myFile.good())
    {
        while (std::getline(myFile, line))
        {
            row.clear();
            stringstream s(line);

            while (std::getline(s, word, ','))
            {
                count++;
                row.push_back(word);
            }
            csv_x1 = stod(row[0]);
            csv_x2 = stod(row[1]);
            csv_y = stoi(row[5]);
            csv_a1 = stod(row[2]);
            csv_a2 = stod(row[3]);
            csv_a3 = stod(row[4]);

            if ((csv_a1 <= 8.0 && csv_a1 >= -8.0) && (csv_a2 <= 8.0 && csv_a2 >= -8.0) &&
                (csv_a3 <= 8.0 && csv_a3 >= -8.0))
            {
                x1.push_back(csv_x1);
                x2.push_back(csv_x2);
                y.push_back(csv_y);
            }
        }
    }

    myFile.close();

    double weight1 = -0.40623956138470096;
    double weight2 = 1.4836620748420632;
    double weight3 = -1.9713767969685267;
    double weight4 = 2.789910806269761;
    double weight_out1 = 8.65565967308743;
    double weight_out2 = -7.968147975662568;
    double bias1 = -9.158888066660062;
    double bias2 = -4.371639350992054;
    double bias_out = 3.708614314955103;

    Plaintext weight1_plain, weight2_plain, weight3_plain, weight4_plain, weight_out1_plain, weight_out2_plain,
        bias1_plain, bias2_plain, bias_out_plain;
    Plaintext input_plain1, input_plain2;
    Ciphertext input_cipher1, input_cipher2, nw_output;
    Ciphertext weight1_cipher, weight2_cipher, weight3_cipher, weight4_cipher, weight_out1_cipher, weight_out2_cipher,
        bias1_cipher, bias2_cipher, bias_out_cipher;

    double scale = pow(2.0, q);
    double bias_scale = pow(2.0, 2 * q);

    time_start = chrono::high_resolution_clock::now();
    encoder.encode(weight1, scale, weight1_plain);
    encoder.encode(weight2, scale, weight2_plain);
    encoder.encode(weight3, scale, weight3_plain);
    encoder.encode(weight4, scale, weight4_plain);
    encoder.encode(weight_out1, scale, weight_out1_plain);
    encoder.encode(weight_out2, scale, weight_out2_plain);
    encoder.encode(bias1, scale, bias1_plain); // -------------- special encoding
    encoder.encode(bias2, scale, bias2_plain);
    encoder.encode(bias_out, bias_scale, bias_out_plain); // -------------------- special encoding
    encoder.encode(x1, scale, input_plain1);
    encoder.encode(x2, scale, input_plain2);
    encryptor.encrypt(input_plain1, input_cipher1);
    encryptor.encrypt(input_plain2, input_cipher2);
    encryptor.encrypt(weight1_plain, weight1_cipher);
    encryptor.encrypt(weight2_plain, weight2_cipher);
    encryptor.encrypt(weight3_plain, weight3_cipher);
    encryptor.encrypt(weight4_plain, weight4_cipher);
    encryptor.encrypt(weight_out1_plain, weight_out1_cipher);
    encryptor.encrypt(weight_out2_plain, weight_out2_cipher);
    encryptor.encrypt(bias1_plain, bias1_cipher);
    encryptor.encrypt(bias2_plain, bias2_cipher);
    encryptor.encrypt(bias_out_plain, bias_out_cipher);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    vector<Ciphertext> nw_weights{ weight1_cipher, weight2_cipher,     weight3_cipher,
                                   weight4_cipher, weight_out1_cipher, weight_out2_cipher };
    vector<Ciphertext> nw_bias{ bias1_cipher, bias2_cipher, bias_out_cipher };
    size_t input_per_layer[2] = { 2, 2 };
    size_t output_per_layer[2] = { 2, 1 };
    size_t no_of_layers = 2;

    Ciphertext nw_input[2] = { input_cipher1, input_cipher2 };

    size_t input_size = (input_cipher1.int_array().size()) / 1024 + (input_cipher2.int_array().size()) / 1024 +
                        (weight1_cipher.int_array().size()) / 1024 + (weight2_cipher.int_array().size()) / 1024 +
                        (weight3_cipher.int_array().size()) / 1024 + (weight4_cipher.int_array().size()) / 1024 +
                        (weight_out1_cipher.int_array().size()) / 1024 + (weight_out2_cipher.int_array().size()) / 1024;
    cout << " + Total size of input ciphertexts to the network is : " << input_size << " KB . " << endl;
    cout << " + Total encoding and encryption time of the input ciphertets is : " << time_diff.count()
         << "microseconds ." << endl;

    time_start = chrono::high_resolution_clock::now();
    neural_network(
        context, nw_input, nw_weights, nw_bias, no_of_layers, input_per_layer, output_per_layer, nw_output, q,
        relin_keys);
    time_end = chrono::high_resolution_clock::now();
    time_diff = chrono::duration_cast<chrono::microseconds>(time_end - time_start);

    Plaintext result_plain;
    vector<double> result_vector;
    decryptor.decrypt(nw_output, result_plain);
    encoder.decode(result_plain, result_vector);

    cout << " + Size of the output ciphertext of neural network is : " << (nw_output.int_array().size()) / 1024
         << " KB . " << endl;

    // calculate the accuracy of the evaluated network predicted output
    double j = 0;
    for (int i = 0; i < y.size(); i++)
    {
        if (result_vector[i] >= 0.5)
        {
            y_pred.push_back(1);
        }
        else
        {
            y_pred.push_back(0);
        }

        if (y[i] == y_pred[i])
            j++;
    }

    cout << endl;
    cout << " + Total time for computation of neural network of 2 layers is : " << time_diff.count()
         << " microseconds. " << endl;
    cout << " + Total no. of input samples for non-linear classsification : " << y.size() << endl;
    cout << " + Total no. of correctly classified samples by the network : " << j << endl;
    double accuracy = (j / 105) * 100;
    cout << " + The validation accuracy is  " << accuracy << " percent ." << endl;
    cout << endl;
}

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

void test_sign_algo1()
{
    EncryptionParameters parms(scheme_type::BFV);
    cout << " Pick an encoder to test the sign function. " << endl;
    cout << "  **************************************** " << endl;
    cout << " Enter 1 for integer encoder " << endl;
    cout << " Enter 2 for batch encoder " << endl;
    cout << endl;
    cout << "  **************************************** " << endl;
    int pick = 0;
    cin >> pick;

    if (!(pick))
    {
        cout << "Invalid option." << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    if (pick = 1)
    {
        cout << endl;
        size_t poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        auto context = SEALContext::Create(parms);
        sign1(context, pick);

        cout << endl;
        poly_modulus_degree = 16384;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        context = SEALContext::Create(parms);
        sign1(context, pick);

        cout << endl;
        poly_modulus_degree = 32768;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        parms.set_plain_modulus(786433);
        context = SEALContext::Create(parms);
        sign1(context, pick);
    }
    else if (pick = 2)
    {
        size_t poly_modulus_degree = 4096;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        auto context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 8192;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 16384;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);

        poly_modulus_degree = 32768;
        parms.set_poly_modulus_degree(poly_modulus_degree);
        parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
        // parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
        parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
        context = SEALContext::Create(parms);
        sign1(context, pick);
    }
}

void test_sign_algo2()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    auto context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    sign2(context);



}

void test_sigmoid()
{
    EncryptionParameters parms(scheme_type::CKKS);

    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 24,24,24,24,24,24,24,24,24}));
    auto context = SEALContext::Create(parms);
    int q = 24;
    sigmoid_setup(context, q);
    
    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 40,40,40,40,40,40,40,40,40 }));
    context = SEALContext::Create(parms);
    q = 40;
    sigmoid_setup(context, q);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 60, 60, 60, 60, 60, 60, 60, 60 }));
    context = SEALContext::Create(parms);
    q = 60;
    sigmoid_setup(context, q);
}


void test_dnn()
{
    EncryptionParameters parms(scheme_type::CKKS);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(
        CoeffModulus::Create(poly_modulus_degree, { 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25 }));
    auto context = SEALContext::Create(parms);
    int q = 25;
    dnn(context, q);


    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(
        CoeffModulus::Create(poly_modulus_degree, { 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                                                    50, 50, 50}));

    context = SEALContext::Create(parms);
    q = 50;
    dnn(context, q);
}



void test_discreteNN()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    auto context = SEALContext::Create(parms);
    discrete(context);

    
    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc192));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 16384;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

    poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree, sec_level_type::tc256));
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    context = SEALContext::Create(parms);
    discrete(context);

}


int main()
{
    while (true)
    {
        cout << " Select the test to run out of below options" << endl;
        cout << " 1. sign function with algorithm1 " << endl;
        cout << " 2. sign function with algorithm2 " << endl;
        cout << " 3. sigmoid function " << endl;
        cout << " 4. deep neural network " << endl;
        cout << " 5. discrete neural network " << endl;

        int select;
        cin >> select;
        switch (select)
        {
        case 1:
            test_sign_algo1();
            break;
        case 2:
            test_sign_algo2();
            break;
        case 3:
            test_sigmoid();
            break;
        case 4:
            test_dnn();
            break;
        case 5:
            test_discreteNN();
            break;
        default:
            cout << " Invalid option " << endl;
        }

    }
    

    return 0;
}