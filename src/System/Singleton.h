#ifndef AGAR_SINGLETON_H
#define AGAR_SINGLETON_H

template <class T>
class Singleton
{
    public:
        /* retrieve singleton only instace */
        static T* getInstance()
        {
            // if not yet present, create new
            if (!m_instance)
                m_instance = new T;
            return m_instance;
        }

    protected:
        Singleton();
        ~Singleton();

    private:
        /* disable copying */
        Singleton(Singleton const&);
        /* disable assignment */
        Singleton& operator = (Singleton const&);
        /* only one instance of singleton class */
        static T* m_instance;
};

/* make all singleton-wrapped classes have their static instance
   pointer null by default, so we knew we have to create new instance */
template <class T> T* Singleton<T>::m_instance = nullptr;

#endif
