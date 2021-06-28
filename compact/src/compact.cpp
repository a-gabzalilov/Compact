#include <vector>
#include <cmath>
#include "../include/ICompact.h"
#include "IVector.h"

namespace {
    class compact: public ICompact
    {
    public:
        compact(IVector * begin, IVector * end, ILogger *logger);
        IVector* getBegin() const override;
        IVector* getEnd() const override;

        iterator* end(IVector const* const step = 0) override;
        iterator* begin(IVector const* const step = 0) override;

        RESULT_CODE isContains(IVector const* const vec, bool& result) const override;
        RESULT_CODE isSubSet(ICompact const* const other,bool& result) const override;
        RESULT_CODE isIntersects(ICompact const* const other, bool& result) const override;

        size_t getDim() const override;
        ICompact* clone() const override;
        class my_iterator: public iterator
        {
        public:
            my_iterator(IVector const * const startPoint, IVector const * const endPoint, IVector * step);

            RESULT_CODE doStep() override;

            IVector* getPoint() const override;

            RESULT_CODE setDirection(IVector const* const dir) override;
        private:
            IVector *currentPoint;
            IVector  *my_step, *my_dir;
            const IVector * const my_startPoint, * const my_endPoint;

        };

    private:
        //std::vector<IVector*> start_vector, end_vector;
        IVector *start_vector, *end_vector;
        ILogger *new_logger;
    };

}

compact::compact(IVector *begin, IVector *end, ILogger *logger): start_vector(begin), end_vector(end), new_logger(logger){
}

IVector *compact::getBegin() const {
    return start_vector->clone();
}

IVector *compact::getEnd() const {
    return end_vector->clone();
}

RESULT_CODE compact::isContains(IVector const *const vec, bool &result) const {
    if (vec == nullptr)
    {
        return RESULT_CODE::BAD_REFERENCE;
    }
    if (vec->getDim() != getDim())
    {
        return RESULT_CODE::WRONG_DIM;
    }
    for (size_t i = 0; i < vec->getDim(); ++i) {
        if (vec->getCoord(i) < start_vector->getCoord(i) || vec->getCoord(i) > end_vector->getCoord(i)) {
            result = false;
            return RESULT_CODE::SUCCESS;
        }
    }
    result = true;
    return RESULT_CODE::SUCCESS;
}

RESULT_CODE compact::isSubSet(ICompact const *const other, bool &result) const {
    if (other == nullptr)
    {
        return RESULT_CODE::BAD_REFERENCE;
    }
    if (other->getDim() != getDim())
    {
        return RESULT_CODE::WRONG_DIM;
    }
    for (int i = 0; i < start_vector->getDim(); ++i) {
        if (start_vector->getCoord(i) > other->getBegin()->getCoord(i) ||
            end_vector->getCoord(i) < other->getEnd()->getCoord(i)) {
            result = false;
            return RESULT_CODE::SUCCESS;
        }
    }
    result = true;
    return RESULT_CODE::SUCCESS;
}

RESULT_CODE compact::isIntersects(ICompact const *const other, bool &result) const {
    if (other == nullptr)
    {
        return RESULT_CODE::BAD_REFERENCE;
    }
    if (other->getDim() != getDim())
    {
        return RESULT_CODE::WRONG_DIM;
    }
    double first_point, second_point;
    for (int i = 0; i < other->getEnd()->getDim(); i++)
    {
        first_point = std::max(other->getBegin()->getCoord(i), start_vector->getCoord(i));
        second_point = std::min(other->getEnd()->getCoord(i), end_vector->getCoord(i));
        if (first_point > second_point)
        {
            result = false;
            return RESULT_CODE::SUCCESS;
        }
    }
    result = true;
    return RESULT_CODE::SUCCESS;
}

size_t compact::getDim() const {
    return start_vector->getDim();
}

ICompact *compact::clone() const {
    return new compact(start_vector->clone(), end_vector->clone(), new_logger);
}

ICompact::iterator* compact::end(IVector const *const step) {
    if (step == nullptr)
    {
        return nullptr;
    }
    for (int i =0; i < step->getDim(); i++)
    {
        if (step->getCoord(i)>0)
            return nullptr;
    }
    return new my_iterator(end_vector, start_vector, step->clone());
}

ICompact::iterator* compact::begin(IVector const *const step) {
    if (step == nullptr)
    {
        return nullptr;
    }
    for (int i =0; i < step->getDim(); i++)
    {
        if (step->getCoord(i)<0)
            return nullptr;
    }
    return new my_iterator(start_vector, end_vector, step->clone());
}

ICompact *ICompact::createCompact(IVector const *const begin, IVector const *const end, ILogger *logger) {
    if (begin == nullptr || end == nullptr)
    {
        logger->log("Vector should not be nullptr", RESULT_CODE::WRONG_ARGUMENT);
        return nullptr;
    }
    if (begin->getDim() != end->getDim())
    {
        logger->log("Begin vector should be the same size as and vector", RESULT_CODE::WRONG_DIM);
        return nullptr;
    }
    for (int i = 0; i < begin->getDim(); i++)
    {
        if (begin->getCoord(i)>end->getCoord(i))
        {
            logger->log("Begin vector's coordinates should be less than end vector's", RESULT_CODE::WRONG_ARGUMENT);
            return nullptr;
        }
    }
    return new compact(begin->clone(), end->clone(), logger);
}

ICompact *ICompact::intersection(ICompact const *const left, ICompact const *const right, ILogger *logger) {
    if (left == nullptr || right == nullptr)
    {
        logger->log("No such compacts", RESULT_CODE::BAD_REFERENCE);
        return nullptr;
    }
    if (left->getBegin()->getDim() != right->getBegin()->getDim())
    {
        logger->log("Left and right compact's dimensions are different", RESULT_CODE::WRONG_DIM);
        return nullptr;
    }
    //checking is there're any similar components
    /*for (int i = 0; i < left->getEnd()->getDim(); i++)
    {
        if (left->getEnd()->getCoord(i) < right->getBegin()->getCoord(i))
            {
            logger->log("Unable to intersect compacts: there're no similar components", RESULT_CODE::NOT_FOUND);
            return nullptr;
            }
    }*/
    IVector *new_begin = left->getEnd()->clone();
    IVector *new_end = left->getEnd()->clone();
    for (int i = 0; i < left->getEnd()->getDim(); i++)
    {
        new_begin->setCoord(i, std::max(left->getBegin()->getCoord(i), right->getBegin()->getCoord(i)));
        new_end->setCoord(i, std::min(left->getEnd()->getCoord(i), right->getEnd()->getCoord(i)));
        if (new_begin->getCoord(i) > new_end->getCoord(i))
        {
            logger->log("Unable to intersect compacts: there're no similar components", RESULT_CODE::NOT_FOUND);
            return nullptr;
        }
    }
    return new compact(new_begin, new_end, logger);
}

ICompact *ICompact::makeConvex(ICompact const* const left, ICompact const* const right, ILogger*logger){
    if (left == nullptr || right == nullptr)
    {
        logger->log("No such compacts", RESULT_CODE::BAD_REFERENCE);
        return nullptr;
    }
    if (left->getBegin()->getDim() != right->getBegin()->getDim())
    {
        logger->log("Left and right compact's dimensions are different", RESULT_CODE::WRONG_DIM);
        return nullptr;
    }
    IVector *new_begin = left->getEnd()->clone();
    IVector *new_end = left->getEnd()->clone();
    for (int i = 0; i < left->getEnd()->getDim(); i++)
    {
        new_begin->setCoord(i, std::min(left->getBegin()->getCoord(i), right->getBegin()->getCoord(i)));
        new_end->setCoord(i, std::max(left->getEnd()->getCoord(i), right->getEnd()->getCoord(i)));
    }
    return new compact(new_begin, new_end, logger);
}

ICompact *ICompact::add(ICompact const *const left, ICompact const *const right, ILogger *logger) {
    if (left == nullptr || right == nullptr)
    {
        logger->log("No such compacts", RESULT_CODE::BAD_REFERENCE);
        return nullptr;
    }
    if (left->getBegin()->getDim() != right->getBegin()->getDim())
    {
        logger->log("Left and right compact's dimensions are different", RESULT_CODE::WRONG_DIM);
        return nullptr;
    }
    int check = 0;
    for (int i = 0; i < left->getEnd()->getDim(); i++)
    {
        if (left->getBegin()->getCoord(i) == right->getBegin()->getCoord(i))
            if (left->getEnd()->getCoord(i) == right->getEnd()->getCoord(i))
                check++;
    }
    if (check < left->getEnd()->getDim()-1)
    {
        bool res = true;
        RESULT_CODE err = left->isSubSet(right, res);
        if(!res) {
            logger->log("Compact union is not convex", RESULT_CODE::BAD_REFERENCE);
            return nullptr;
        }
    }
    IVector *new_begin = left->getEnd()->clone();
    IVector *new_end = left->getEnd()->clone();
    for (int i = 0; i < left->getEnd()->getDim(); i++)
    {
        new_begin->setCoord(i, std::min(left->getBegin()->getCoord(i), right->getBegin()->getCoord(i)));
        new_end->setCoord(i, std::max(left->getEnd()->getCoord(i), right->getEnd()->getCoord(i)));
    }
    return new compact(new_begin, new_end, logger);
}

//ITERATOR
compact::my_iterator::my_iterator(IVector const *const startPoint, IVector const *const endPoint, IVector *step):
        currentPoint(startPoint->clone()), my_startPoint(startPoint), my_endPoint(endPoint), my_step(step), my_dir(nullptr)
{}

RESULT_CODE compact::my_iterator::doStep() {
    if (my_startPoint == nullptr || my_endPoint == nullptr || my_step == nullptr)
    {
        return RESULT_CODE::WRONG_ARGUMENT;
    }
    //checking the way we going
    int way = 1;
    if (my_startPoint->getCoord(0)>my_endPoint->getCoord(0))
        way = -1;
    //starting with the first component of direction vector, if it exists
    int component = 0;
    while (component != my_startPoint->getDim())
    {
        int my_component = component;
        if (my_dir)
            my_component = my_dir->getCoord(component);
        if (way*(currentPoint->getCoord(my_component)+my_step->getCoord(my_component))<=way*my_endPoint->getCoord(my_component))
        {
            currentPoint->setCoord(my_component, currentPoint->getCoord(my_component)+my_step->getCoord(my_component));
            return RESULT_CODE::SUCCESS;
        }
        currentPoint->setCoord(my_component, my_startPoint->getCoord(my_component));
        component += 1;
    }
    return RESULT_CODE::OUT_OF_BOUNDS;
}

RESULT_CODE compact::my_iterator::setDirection(IVector const *const dir) {
    if (dir == nullptr)
        return RESULT_CODE::BAD_REFERENCE;
    if (dir->getDim() != my_startPoint->getDim())
        return RESULT_CODE::WRONG_DIM;
    int check_component = 0;
    for (int i = 0; i<dir->getDim(); i++){
        if (dir->getCoord(i)==check_component)
            check_component++;
        }
    if (check_component != dir->getDim()-1)
        return RESULT_CODE::WRONG_ARGUMENT;
    else
        {
            for (int i = 0; i< currentPoint->getDim(); i++)
            {
                currentPoint->setCoord(i, my_startPoint->getCoord(i));
            }
            my_dir = dir->clone();
            return RESULT_CODE::SUCCESS;
        }
}

IVector *compact::my_iterator::getPoint() const {
    return currentPoint->clone();
}

ICompact::~ICompact() {}


