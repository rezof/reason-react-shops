let component = ReasonReact.statelessComponent("Home");

let make = _children => {
  ...component,
  render: _self => <p> (ReasonReact.string("home")) </p>
};