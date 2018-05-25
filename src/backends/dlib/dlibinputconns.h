/*copyright (c) 2016 Emmanuel Benazera
 * Author: Emmanuel Benazera <beniz@droidnik.fr>
 *
 * This file is part of deepdetect.
 *
 * deepdetect is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * deepdetect is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with deepdetect.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DLIBINPUTCONNS_H
#define DLIBINPUTCONNS_H

#include "imginputfileconn.h"
#include "opencv2/opencv.hpp"
#include "dlib/data_io.h"
#include "dlib/image_io.h"
#include "dlib/image_transforms.h"
#include "dlib/image_processing.h"
#include "dlib/opencv/to_open_cv.h"
#include "dlib/opencv/cv_image.h"


#include "inputconnectorstrategy.h"
#include "ext/base64/base64.h"

namespace dd {
    class DlibInputInterface {
    public:
        DlibInputInterface() {}

        DlibInputInterface(const DlibInputInterface &tii)
                : _dv(tii._dv), _ids(tii._ids) {}

        ~DlibInputInterface() {}

    public:
        // parameters common to all Dlib input connectors
        std::vector<dlib::matrix<dlib::rgb_pixel>> _dv;
        std::vector<dlib::matrix<dlib::rgb_pixel>> _dv_test;
        std::vector<std::string> _ids; // input ids (eg. Image Ids).
    };

    class ImgDlibInputFileConn : public ImgInputFileConn, public DlibInputInterface {
    public:
        ImgDlibInputFileConn()
                : ImgInputFileConn() {
            reset_dv();
        }

        ImgDlibInputFileConn(const ImgDlibInputFileConn &i)
                : ImgInputFileConn(i), DlibInputInterface(i), _mean(i._mean), _std(i._std) {}

        ~ImgDlibInputFileConn() {}

        int channels() const {
            if (_bw) return 1;
            else return 3; // RGB
        }

        int height() const {
            return _height;
        }

        int width() const {
            return _width;
        }

        int batch_size() const {
            if (!_dv.empty())
                return _dv.size();
            else return ImgInputFileConn::batch_size();
        }

        int test_batch_size() const {
            if (!_dv_test.empty())
                return _dv_test.size();
            else return ImgInputFileConn::test_batch_size();
        }

        void init(const APIData &ad) {
            ImgInputFileConn::init(ad);
            if (ad.has("mean"))
                _mean = ad.get("mean").get<double>();
            if (ad.has("std"))
                _std = ad.get("std").get<double>();
        }

        void transform(const APIData &ad) {
            try {
                ImgInputFileConn::transform(ad);
            }
            catch (InputConnectorBadParamException &e) {
                throw;
            }

            APIData ad_param = ad.getobj("parameters");
            if (ad_param.has("input")) {
                APIData ad_input = ad_param.getobj("input");
                if (ad_input.has("mean"))
                    _mean = ad_input.get("mean").get<double>();
                if (ad_input.has("std"))
                    _std = ad_input.get("std").get<double>();
            }

            for (size_t i = 0; i < _images.size(); i++) {
//                tensorflow::Tensor input_tensor(tensorflow::DT_FLOAT,
//                                                tensorflow::TensorShape({1, _height, _width, channels()}));
//                auto input_tensor_mapped = input_tensor.tensor<float, 4>();
//
//                cv::Mat CImage = std::move(this->_images.at(i));
//                cv::Mat Image;
//                CImage.convertTo(Image, CV_32FC1);
//                cv::Mat Image2;
//                cv::cvtColor(Image, Image2, CV_BGR2RGB); // because OpenCV defaults to BGR
//                Image = (Image2 - _mean) / _std;
//                const float *source_data = (float *) Image.data;
//
//                // copying the data into the corresponding tensor
//                for (int y = 0; y < height(); ++y) {
//                    const float *source_row = source_data + (y * width() * channels());
//                    for (int x = 0; x < width(); ++x) {
//                        const float *source_pixel = source_row + (x * channels());
//                        for (int c = 0; c < channels(); ++c) {
//                            const float *source_value = source_pixel + c;
//                            input_tensor_mapped(0, y, x, c) = *source_value;
//                        }
//                    }
//                }
//
//                _dv.push_back(input_tensor);
//                _ids.push_back(_uris.at(i));
                dlib::matrix<dlib::rgb_pixel> inputImg;
                dlib::assign_image(inputImg, dlib::cv_image<dlib::rgb_pixel>(_images.at(i)));
                _dv.push_back(inputImg);
                _ids.push_back(_uris.at(i));
            }
            _images.clear();
        }

        std::vector<dlib::matrix<dlib::rgb_pixel>> get_dv(const int &num) {
            int i = 0;
            std::vector<dlib::matrix<dlib::rgb_pixel>> dv;
            while(_dt_vit!=_dv.end() && i < num) {
                dv.push_back((*_dt_vit));
                ++i;
                ++_dt_vit;
            }
            return dv;
        }

        void reset_dv()
        {
            _dt_vit = _dv.begin();
        }

    public:
        int _mean = 128;
        int _std = 128;
        std::vector<dlib::matrix<dlib::rgb_pixel>>::const_iterator _dt_vit;

    };

}

#endif

