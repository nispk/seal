#include "performance_test.h"
#include <fstream>
#include <iostream>
#include "sign_function.h"


using namespace std;
using namespace seal;

/*
This method computes sign function using BFV scheme.
We have implemented algorithm1 in this method.
*/

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
    cout << " + Noise budget in denominator ciphertext is : " << decryptor.invariant_noise_budget(denominator)
         << " bits . " << endl;
    cout << " + Numerator storage space is : " << (numerator.int_array().size()) / 1024 << " KB . " << endl;

    cout << " + Denominator storage space is : " << (denominator.int_array().size()) / 1024 << " KB ." << endl;

    cout << " + Average computation time for algorithm 1 is : " << (time_diff.count()) / count << " microseconds "
         << endl;
    cout << endl;
}