/*!
 *  Copyright (c) 2014 by Contributors
 * \file tensor_container.h
 * \brief tensor container that does memory allocation and resize like STL
 * \author Tianqi Chen
 */
#ifndef MSHADOW_TENSOR_CONTAINER_H_
#define MSHADOW_TENSOR_CONTAINER_H_
#include "./tensor.h"
#include "./io.h"

namespace mshadow {
/*!
 * \brief tensor container that does memory allocation and resize like STL,
 *        use it to save the lines of FreeSpace in class.
 *        Do not abuse it, efficiency can come from pre-allocation and no re-allocation
 *
 * \tparam Device which device the tensor is on
 * \tparam dimension dimension of the tensor
 */
template<typename Device, int dimension, typename DType = default_real_t>
class TensorContainer: public Tensor<Device, dimension, DType> {
 public:
  /*! 
   * \brief constructor 
   * \param pad whether use padding alignment in space allocation
   */
  explicit TensorContainer(bool pad = MSHADOW_ALLOC_PAD) {
    this->pad_ = pad;
    this->dptr_ = data_.dptr_ = NULL;
    this->shape_[0] = 0;
    this->stride_ = 0;
    this->data_.stride_ = 0;
    this->data_.shape_[0] = 0;
  }
  /*! 
   * \brief constructor 
   * \param shape intial shape
   */
  explicit TensorContainer(const Shape<dimension> &shape) {
    this->pad_ = MSHADOW_ALLOC_PAD;
    data_.dptr_ = NULL;
    this->AllocByShape(shape);
  }
  /*! 
   * \brief constructor 
   * \param shape intial shape
   * \param initv intial value
   */
  explicit TensorContainer(const Shape<dimension> &shape, DType initv) {
    this->pad_ = MSHADOW_ALLOC_PAD;
    data_.dptr_ = NULL;
    this->AllocByShape(shape);
    (*this) = initv;
  }
  ~TensorContainer(void) {
    this->FreeSpace();
  }
  /*! 
   * \brief resize the container to given shape, content is NOT preserved
   * \param shape target shape
   */
  inline void Resize(const Shape<dimension> &shape) {
    Shape<2> s2 = shape.FlatTo2D();
    if (s2.shape_[1] > data_.stride_ || s2.shape_[0] > data_.size(0)) {
      this->AllocByShape(shape);
    } else {
      this->shape_ = shape;
      if (this->pad_) {
        this->stride_ = data_.stride_;
      } else {
        this->stride_ = s2.shape_[1];
      }
    }
  }
  /*! 
   * \brief resize the container to given shape, and initialize, content is NOT preserved
   * \param shape target shape
   * \param initv initialization value
   */
  inline void Resize(const Shape<dimension> &shape, DType initv) {
    this->Resize(shape);
    (*this) = initv;
  }
  /*! \brief set whether padding is allowed in tensor */
  inline void set_pad(bool pad) {
    this->pad_ = pad;
  }
  /*! 
   * \brief save by binary format
   * \param fo output binary stream
   * \tparam TStream type of stream, need to support Read, Write, one example is utils::IStream.
   */
  template<typename TStream>
  inline void SaveBinary(TStream &fo) const {
    mshadow::SaveBinary(fo, *this);
  }
  /*! 
   * \brief load by binary format, a temp Tensor<cpu,dim> storage will be allocated
   * \param fi input binary stream
   * \tparam TStream type of stream, need to support Read, Write, one example is utils::IStream.
   */
  template<typename TStream>
  inline void LoadBinary(TStream &fi) {
    Tensor<cpu, dimension, DType> tmp;
    mshadow::LoadBinary(fi, &tmp, false);
    this->Resize(tmp.shape_);
    Stream<Device> stream;
    Copy(*this, tmp, &stream);
    mshadow::FreeSpace(&tmp);
  }
  /*!\brief functions to fit expression template */
  inline Tensor<Device, dimension, DType> &operator=(DType s) {
    return this->__assign(s);
  }
  /*!\brief functions to fit expression template */
  template<typename E>
  inline Tensor<Device, dimension, DType> &
  operator=(const expr::Exp<E, DType, expr::type::kMapper> &exp) {
    return this->__assign(exp);
  }
  /*!\brief functions to fit expression template */
  template<typename E>
  inline Tensor<Device, dimension, DType> &
  operator=(const expr::Exp<E, DType, expr::type::kChainer> &exp) {
    return this->__assign(exp);
  }
  /*!\brief functions to fit expression template */
  template<typename E>
  inline Tensor<Device, dimension, DType> &
  operator=(const expr::Exp<E, DType, expr::type::kComplex> &exp) {
    return this->__assign(exp);
  }

 private:
  /*! \brief whether we do padding in the space */
  bool pad_;
  /*! \brief the shape of data_ is actually current data space */
  Tensor<Device, 2, DType> data_;
  // freespace
  inline void FreeSpace(void) {
    if (data_.dptr_ != NULL) {
      mshadow::FreeSpace(&data_);
      data_.dptr_ = this->dptr_ = NULL;
    }
  }
  inline void AllocByShape(const Shape<dimension>& shape) {
    if (data_.dptr_ != NULL) this->FreeSpace();
    data_.shape_ = shape.FlatTo2D();
    mshadow::AllocSpace(&data_, pad_);
    this->dptr_ = data_.dptr_;
    this->shape_ = shape;
    if (this->pad_) {
      this->stride_ = data_.stride_;
    } else {
      this->stride_ = data_.size(1);
    }
  }
};
}  // namespace mshadow
#endif  // MSHADOW_TENSOR_CONTAINER_H_
