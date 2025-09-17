#ifndef FACTORY_HPP
#define FACTORY_HPP

template <typename Interface>
class Factory
{
public:
  virtual ~Factory() {}
  virtual Interface *create() = 0;
};

#endif
