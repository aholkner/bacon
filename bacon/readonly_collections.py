import collections
class ReadOnlyDict(collections.MutableMapping):
    def __init__(self, store):
        self.store = store
        
    def __getitem__(self, key):
        return self.store[key]

    def __setitem__(self, key, value):
        raise TypeError('Cannot modify ReadOnlyDict')

    def __delitem__(self, key):
        raise TypeError('Cannot modify ReadOnlyDict')

    def __iter__(self):
        return iter(self.store)

    def __len__(self):
        return len(self.store)

    def __str__(self):
        return 'ReadOnlyDict(%s)' % self.store

    def __repr__(self):
        return 'ReadOnlyDict(%r)' % self.store