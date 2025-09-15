#ifndef FACTORY_HPP
#define FACTORY_HPP

template <typename Interface>
class Factory
{
public:
  virtual Interface *create() = 0;
};

#endif