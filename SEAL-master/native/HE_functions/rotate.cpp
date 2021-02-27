#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include "examples.h"
#include "math.h"
#include <memory>
#include <seal/seal.h>

using namespace std;
using namespace seal;

std::unique_ptr<seal::PublicKey> pk;
std::unique_ptr<seal::SecretKey> sk;
std::unique_ptr<seal::RelinKeys> rk;

vector<Ciphertext> rotate_right(shared_ptr<SEALContext> context, vector<Ciphertext> x, int64_t s)
{
    Decryptor decryptor(context, *sk);
    BatchEncoder batchEncoder(context);

    for (int64_t i = 0; i < s; i++)
    {
        x.insert(x.begin(), x.back());
        x.pop_back();
    }
    cout << " vector after " << s << "  rotations " << endl;
    for (int q = 0; q <= 16; q++)
    {
        Plaintext plain1;
        decryptor.decrypt(x[q], plain1);
        std::vector<int64_t> vec;
        batchEncoder.decode(plain1, vec);
        cout << vec.at(0);
        cout << endl;
        plain1.release();
    }
    return x;
}


void rotate_mult(shared_ptr<SEALContext> context, Ciphertext input)
{
    Encryptor encryptor(context, *pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, *sk);
    BatchEncoder batchEncoder(context);
    vector<Plaintext> x_plain;
    vector<Ciphertext> x_enc;
    vector<Plaintext> func_indices;
    Ciphertext temp, enc;
    int64_t n = 4;
    int64_t N = pow(2, n);
    int64_t max_input_range = 8;

    func_indices.clear();
    x_enc.clear();
    x_plain.clear();
   
    Plaintext decrp;
    decryptor.decrypt(input, decrp);
    std::vector<int64_t> vec;
    batchEncoder.decode(decrp, vec);
    cout << " The input to this function is " << vec.at(0) << endl;
    cout << endl;

    Plaintext max_input_plain;
    std::vector<int64_t> mir = {max_input_range};
    batchEncoder.encode(mir, max_input_plain);
    evaluator.add_plain(input, max_input_plain, temp);
    max_input_plain.release();

    Plaintext dec;
    decryptor.decrypt(temp, dec);
    vec.clear();
    batchEncoder.decode(dec, vec);
    cout << " temp value after adding max_input_plain " << vec.at(0) << endl;
    
    cout << " Compute indices vector,test input vector and the encrypted x vector" << endl;
    for (int j = 0; j <= N; j++)
    {
        x_enc.push_back(temp);  // = 2
        vec.clear();
        vec.push_back(j);
        Plaintext ptxt;
        batchEncoder.encode(vec, ptxt);
        // func_indices.push_back(batchEncoder.encode(j)); // = 0, 1, ..., 16
        func_indices.push_back(ptxt); // = 0, 1, ..., 16
        evaluator.sub_plain_inplace(x_enc[j], func_indices[j]); // 2-0, 2-1, 2-2, ... , 2-16
    }

    vector<Ciphertext> rot_x;
    for (size_t s = 0; s < n; s++)
    {
        rot_x.clear();
        rot_x = rotate_right(context, x_enc, pow(2, s));
        Plaintext a1, a2, a3;
        cout << endl;
        cout << " multiplication " << endl;
        for (int i = 0; i <= N; i++)
        {
            decryptor.decrypt(x_enc[i], a1);
            decryptor.decrypt(rot_x[i], a2);
            evaluator.multiply_inplace(x_enc[i], rot_x[i]);
            evaluator.relinearize_inplace(x_enc[i], *rk);
            decryptor.decrypt(x_enc[i], a3);

            std::vector<int64_t> a1_val;
            batchEncoder.decode(a1, a1_val);
            std::vector<int64_t> a2_val;
            batchEncoder.decode(a2, a2_val);
            std::vector<int64_t> a3_val;
            batchEncoder.decode(a3, a3_val);
            cout << a1_val.at(0) << " * " << a2_val.at(0) << "  =  " << a3_val.at(0) << endl;
        }
    

        Plaintext decd;
        cout << endl;
        cout << " rotation input is " << endl;
        for (int i = 0; i <= N; i++)
        {
            decryptor.decrypt(x_enc[i], decd);
            vec.clear();
            batchEncoder.decode(decd, vec);
            cout << vec.at(0) << endl;
        }
        cout << endl;
    }

    cout << " Noise in product x_enc vector is " << endl;
    for (int i = 0; i <= N; i++)
    {
        cout << decryptor.invariant_noise_budget(x_enc[i]) << endl;
        cout << endl;
    }

    x_plain.clear();
    x_enc.clear();
    func_indices.clear();
    temp.release();
    enc.release();

    decrp.release();
}

int main()
{
    EncryptionParameters parms(scheme_type::BFV);
    size_t poly_modulus_degree = 32768;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::BFVDefault(poly_modulus_degree));
    // parms.set_plain_modulus(12289);
    parms.set_plain_modulus(PlainModulus::Batching(poly_modulus_degree, 20));
    // parms.set_plain_modulus(8192);
    auto context = SEALContext::Create(parms);
    cout << "Set encryption parameters and print" << endl;
    print_parameters(context);

    KeyGenerator keygen(context);
    pk = std::make_unique<seal::PublicKey>(keygen.public_key());
    sk = std::make_unique<seal::SecretKey>(keygen.secret_key());
    rk = std::make_unique<seal::RelinKeys>(keygen.relin_keys_local());
    Encryptor encryptor(context, *pk);
    Evaluator evaluator(context);
    Decryptor decryptor(context, *sk);
    BatchEncoder encoder(context);
    
    int64_t v1 = -2;
    int64_t v2 = 3;
    Plaintext v_plain1, v_plain2;
    Ciphertext v_enc1, v_enc2;
    std::vector<int64_t> val1 = {v1};
    std::vector<int64_t> val2 = {v2};
    encoder.encode(val1, v_plain1);
    encoder.encode(val2, v_plain2);
    encryptor.encrypt(v_plain2, v_enc2);
    encryptor.encrypt(v_plain1, v_enc1);
    evaluator.multiply_inplace(v_enc1, v_enc2);
    evaluator.relinearize_inplace(v_enc1, *rk);
    rotate_mult(context, v_enc1);
    
    return 0;
}
