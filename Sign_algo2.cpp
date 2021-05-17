#include "performance_test.h"
#include <iostream>
#include "sign_function.h"

using namespace std;
using namespace seal;




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
    // cin >> max_input_range;
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

    cout << " + Sign function lookup table size is : " << (func_enc[0].int_array().size()) * func_enc.size() / 1024
         << " KB . " << endl;
    cout << " + Sign function preamble vector size is : " << (preamble[0].int_array().size()) * preamble.size() / 1024
         << " KB . " << endl;

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
