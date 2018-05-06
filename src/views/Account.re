let component = ReasonReact.statelessComponent("Account");

let make = _children => {
  ...component,
  render: _self => <p> (ReasonReact.string("account")) </p>
};