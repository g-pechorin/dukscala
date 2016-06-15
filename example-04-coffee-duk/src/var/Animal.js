var Animal;

behave(Animal = (function() {
  function Animal() {
    this.name = 'Nite-Nite';
  }

  Animal.prototype.onAttach = function() {
    return print(this.name + ' is alert');
  };

  Animal.prototype.move = function(meters) {
    return print(this.name + (" moved " + meters + "m."));
  };

  return Animal;

})());
