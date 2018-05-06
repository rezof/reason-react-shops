let component = ReasonReact.statelessComponent("NotFound");

let make = _children => {
  ...component,
  render: _self =>
    <div className="wrapper">
      <p className="not-found"> (ReasonReact.string("404")) </p>
    </div>
};