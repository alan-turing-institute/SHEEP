import sys
import uuid
if sys.version_info[0] < 3:
    __VERSION__ = 2
else:
    __VERSION__ = 3


class variables(object):
    def __init__(self, name, nb=1, randomize_name=0):
        self.name = name
        if randomize_name > 0:
            self.name = self.name + uuid.uuid4().hex[:randomize_name].upper()

        self.nb = nb
        if __VERSION__ == 2:
            assert isinstance(name, basestring)
        else:
            assert isinstance(name, str)

    def __str__(self):
        return self.name

    def __add__(self, x):
        return str(self) + x

    def __radd__(self, x):
        return x + str(self)


class enc_vec(object):
    def __init__(self, name, nb=None, lst=None, randomize_name=4,
                 name_list=None):
        self.randomize_name = randomize_name
        self.name = name
        self._lst = lst
        self.nb = nb
        assert (not (self.nb is None and self._lst is None)
                ), "Need to provide something"
        try:
            assert(self._lst is None or isinstance(self._lst, list))
        except Exception:
            raise NotImplementedError
        self.create(name_list)
        self.name_list = self.get_variables()

    def create(self, name_list):
        '''
        An yet unbuilt vec constructor
        '''
        if name_list is None:
            self._lst = [variables(name=self.name + str(idx),
                                   randomize_name=self.randomize_name)
                         for idx in range(self.nb)]
        else:
            assert(len(name_list) == self.nb), "Name list \
            has differnt length " + \
                str(len(name_list)) + " , " + str(self.nb)
            self._lst = [variables(name=name_list[idx])
                         for idx in range(self.nb)]

    def get_input_dict(self, inp_var):
        '''
        Input : A list of valyues
        Output : A dictionary
        =========================
        Creates the input dictionary
        for each variable in the same order
        as _lst. This is necessary as some lists can
        have difficult names
        '''
        assert(isinstance(inp_var, list)), "Input should be a list"
        assert(len(inp_var) == self.nb), "length should be the same as vec"
        inp_dict = {'TRUE': 1, 'FALSE': 0}
        for idx in range(self.nb):
            inp_dict[self._lst[idx].name] = inp_var[idx]
        return inp_dict

    def get_variables(self):
        return [self._lst[idx].name for idx in range(self.nb)]

    def __getitem__(self, item):
        '''
        Index an item
        '''
        target_lst = self._lst[item]
        if isinstance(item, slice):
            slice_length = len(target_lst)
            return enc_vec(name=self.name,
                           nb=slice_length,
                           lst=target_lst,
                           name_list=self.name_list[item])
        else:
            return target_lst

    def __setitem__(self, key, value):
        self._lst[key] = value

    def __add__(self, next_list):
        '''
        Concatenate two lists
        '''
        assert isinstance(next_list, enc_vec)
        new_name = self.name + '_' + next_list.name
        randomize_name = max(self.randomize_name, next_list.randomize_name)
        new_nb = self.nb + next_list.nb
        new_lst = self._lst + next_list._lst
        new_vec = enc_vec(name=new_name,
                          nb=new_nb,
                          lst=new_lst,
                          randomize_name=randomize_name,
                          name_list=self.name_list + next_list.name_list)
        return new_vec

    def __len__(self):
        return self.nb

    def append(self, obj):
        '''
        Append a variable at the end
        '''
        assert isinstance(
            obj, variables), "I'm not sure what you're trying to append here"
        self.nb += 1
        self._lst.append(obj)

    def push(self, obj):
        '''
        Append a variable at the beginning
        '''
        assert isinstance(
            obj, variables), "I'm not sure what you're trying to append here"
        self.nb += 1
        self.name_list = [obj.name] + self.name_list
        self._lst = [obj] + self._lst


class enc_mat(object):
    def __init__(self, name, size=None, randomize_name=4,
                 name_list=None):
        self.randomize_name = randomize_name
        self.name = name
        self.size = size
        self.nb = size[0] * size[1]
        assert (not (self.nb is None and self._lst is None)
                ), "Need to provide something"
        self._lst = []
        self.name_list = []
        self.create(name_list)

    def create(self, name_list):
        '''
        An yet unbuilt vec constructor
        '''
        if name_list is None:
            for row_idx in range(self.size[0]):
                self._lst.append([variables(
                    name=self.name + str(row_idx) + str(col_idx),
                    randomize_name=self.randomize_name)
                    for col_idx in range(self.size[1])])
                self.name_list.append([var.name for var in self._lst[-1]])
        else:
            assert(len(name_list) == self.size[0])
            assert(len(name_list[0]) == self.size[1]), "Name list \
            has differnt length " + \
                str(len(name_list[0])) + " , " + str(self.nb)
            for row_idx in range(self.size[0]):
                self._lst.append([variables(name=name_list[row_idx][col_idx])
                                  for col_idx in range(self.size[1])])

    def get_input_dict(self, inp_var):
        '''
        Input : A list of valyues
        Output : A dictionary
        =========================
        Creates the input dictionary
        for each variable in the same order
        as _lst. This is necessary as some lists can
        have difficult names
        '''
        assert(isinstance(inp_var, list)), "Input should be a list"
        assert(len(inp_var) == self.size[0] * self.size[1]
               ), str(len(inp_var)) + " ~ " + str(self.size[0] * self.size[1])
        inp_dict = {'TRUE': 1, 'FALSE': 0}
        var_list = self.get_variables()
        for idx, var in enumerate(var_list):
            inp_dict[var] = inp_var[idx]
        return inp_dict

    def get_variables(self):
        var_list = []
        for row_idx in range(self.size[0]):
            var_list = var_list + \
                [self._lst[row_idx]
                    [col_idx].name for col_idx in range(self.size[1])]
        return var_list

    def __getitem__(self, item):
        '''
        Index an item
        '''
        assert not isinstance(item, slice), "NO advanced slicing"
        return enc_vec(name=self.name + '_sl_' + str(item),
                       name_list=self.name_list[item], randomize_name=0,
                       nb=len(self.name_list[item]))

    def __setitem__(self, key, value):
        self._lst[key[0]][key[1]] = value

    def __add__(self, next_list):
        '''
        Concatenate two mats
        '''
        try:
            raise NotImplementedError
        except NotImplementedError:
            print("SORRY! Can't add enc mats")
            sys.exit()

    def __len__(self):
        return self.size[0]

    def append(self, obj):
        '''
        Append a variable at the end
        '''
        try:
            raise NotImplementedError
        except NotImplementedError:
            print("SORRY! Can't append to enc mats")
            sys.exit()

    def push(self, obj):
        '''
        Append a variable at the beginning
        '''
        try:
            raise NotImplementedError
        except NotImplementedError:
            print("SORRY! Can't push to enc mats")
            sys.exit()
