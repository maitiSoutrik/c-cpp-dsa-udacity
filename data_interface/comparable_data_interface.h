#ifndef COMPARABLEDATAINTERFACE_H
#define COMPARABLEDATAINTERFACE_H

class ComparableDataWrapperInterface {
public:
    virtual ~ComparableDataWrapperInterface() = default;

    // Method to get the value for comparison
    virtual int getValue() const = 0;

    // Comparison methods for equality and ordering
    virtual bool isEqual(const ComparableDataWrapperInterface& other) const = 0;
    virtual bool isLessThan(const ComparableDataWrapperInterface& other) const = 0;
};

#endif